#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ListenUtil.h"
#include "Epoll.h"
#include "ThreadPool.h"
#include "TimerQueue.h"
#include "Logging.h"

#include <iostream>
#include <functional> // bind
#include <cassert> // assert
#include <cstring> // bzero
 
#include <unistd.h> // close, read
#include <sys/socket.h> // accept
#include <arpa/inet.h> // sockaddr_in
#include <sys/types.h>

HttpServer::HttpServer(int port, int numThread) 
    : isRun(true),
      port_(port),
      listenFd_(utils::createListenFd(port_)),
      listenRequest_(new HttpRequest(listenFd_)),
      epoll_(new Epoll()),
      threadPool_(new ThreadPool(numThread)),
      TimerQueue_(new TimerQueue())
{
    assert(listenFd_ >= 0);
}

HttpServer::~HttpServer(){
    LOG << "~HttpServer()";
}

void HttpServer::stop(){
    isRun = false;
}

void HttpServer::run(){

    // 注册监听套接字到epoll（可读事件，ET模式）
    epoll_ -> add(listenFd_, listenRequest_.get(), (EPOLLIN | EPOLLET));
    // 注册新连接回调函数
    epoll_ -> setOnConnection(std::bind(&HttpServer::__acceptConnection, this));
    // 注册关闭连接回调函数
    epoll_ -> setOnCloseConnection(std::bind(&HttpServer::__closeConnection, this, std::placeholders::_1));
    // 注册请求处理回调函数
    epoll_ -> setOnRequest(std::bind(&HttpServer::__doRequest, this, std::placeholders::_1));
    // 注册响应处理回调函数
    epoll_ -> setOnResponse(std::bind(&HttpServer::__doResponse, this, std::placeholders::_1));

    // 事件循环
    while(isRun) {
        int timeMS = TimerQueue_ -> getNextExpireTime();
        // 等待事件发生
        int eventsNum = epoll_ -> wait(timeMS);

        if(eventsNum > 0) {
            // 分发事件处理函数
            epoll_ -> handleEvent(listenFd_, threadPool_, eventsNum);
        }
        TimerQueue_ -> handleExpireTimers();   
    }
    /*关闭服务器前的准备工作*/
    std::this_thread::sleep_for(std::chrono::seconds(3));
    exit(1);
}

// ET
void HttpServer::__acceptConnection(){

    while(1) {
        int acceptFd = ::accept4(listenFd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(acceptFd == -1) {
            //可能收到Interrupt signal
            if(errno == EAGAIN)
                break;
            LOG << "[HttpServer::__acceptConnection] accept : " << strerror(errno) << "\n";
            break;
        }
        // 为新的连接套接字分配HttpRequest资源
        HttpRequest* request = new HttpRequest(acceptFd);
        TimerQueue_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&HttpServer::__closeConnection, this, request));
        // 注册连接套接字到epoll（可读，边缘触发，保证任一时刻只被一个线程处理）
        epoll_ -> add(acceptFd, request, (EPOLLIN | EPOLLONESHOT));
    }
}

void HttpServer::__closeConnection(HttpRequest* request){

    int fd = request -> fd();
    if(request -> isWorking()) {
        return;
    }
    LOG << "[HttpServer::__closeConnection] connect fd = " << fd << "is closed\n";
    TimerQueue_ -> delTimer(request);
    epoll_ -> del(fd, request, 0);
    // 释放该套接字占用的HttpRequest资源，在析构函数中close(fd)
    delete request;
    request = nullptr;
}

// LT模式
void HttpServer::__doRequest(HttpRequest* request){

    TimerQueue_ -> delTimer(request);
    assert(request != nullptr);
    int fd = request -> fd();

    int readErrno;
    int nRead = request -> read(&readErrno);

    // read返回0表示客户端断开连接
    if(nRead == 0) {
        request -> setNoWorking();
        __closeConnection(request);
        return; 
    }

    // 非EAGAIN错误，断开连接
    if(nRead < 0 && (readErrno != EAGAIN)) {
        request -> setNoWorking();
        __closeConnection(request);
        return; 
    }

    // EAGAIN错误则释放线程使用权，并监听下次可读事件epoll_ -> mod(...)
    if(nRead < 0 && readErrno == EAGAIN) {
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLONESHOT));
        request -> setNoWorking();
        TimerQueue_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&HttpServer::__closeConnection, this, request));
        return;
    }

    // 解析报文，出错则断开连接
    if(!request -> parseRequest()) {
        // 发送400报文
        HttpResponse response(400, "", false);
        request -> appendOutBuffer(response.makeResponse());

        // 立刻关闭连接了，所以就算没写完也只能写一次
        int writeErrno;
        request -> write(&writeErrno);
        request -> setNoWorking();
        __closeConnection(request); 
        return; 
    }

    // 解析完成
    if(request -> parseFinish()) {
        HttpResponse response(200, request -> getPath(), request -> keepAlive(), request->getUsr(), request->getPwd(), request->getTmp(), request->getMethod());
        request -> appendOutBuffer(response.makeResponse());
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
    }
}

// LT模式
void HttpServer::__doResponse(HttpRequest* request){

    TimerQueue_ -> delTimer(request);
    assert(request != nullptr);
    int fd = request -> fd();

    int toWrite = request -> writableBytes();

    if(toWrite == 0) {
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLONESHOT));
        request -> setNoWorking();
        TimerQueue_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&HttpServer::__closeConnection, this, request));
        return;
    }

    int writeErrno;
    int ret = request -> write(&writeErrno);

    if(ret < 0 && writeErrno == EAGAIN) {
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
        return;
    }

    // 非EAGAIN错误，断开连接
    if(ret < 0 && (writeErrno != EAGAIN)) {
        request -> setNoWorking();
        __closeConnection(request);
        return; 
    }

    if(ret == toWrite) {
        if(request -> keepAlive()) {
            request -> resetParse();
            epoll_ -> mod(fd, request, (EPOLLIN | EPOLLONESHOT));
            request -> setNoWorking();
            TimerQueue_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&HttpServer::__closeConnection, this, request));
        } else {
            request -> setNoWorking();
            __closeConnection(request);
        }
        return;
    }

    epoll_ -> mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
    request -> setNoWorking();
    TimerQueue_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&HttpServer::__closeConnection, this, request));
    return;
}
