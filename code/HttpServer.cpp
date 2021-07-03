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

    // ע������׽��ֵ�epoll���ɶ��¼���ETģʽ��
    epoll_ -> add(listenFd_, listenRequest_.get(), (EPOLLIN | EPOLLET));
    // ע�������ӻص�����
    epoll_ -> setOnConnection(std::bind(&HttpServer::__acceptConnection, this));
    // ע��ر����ӻص�����
    epoll_ -> setOnCloseConnection(std::bind(&HttpServer::__closeConnection, this, std::placeholders::_1));
    // ע��������ص�����
    epoll_ -> setOnRequest(std::bind(&HttpServer::__doRequest, this, std::placeholders::_1));
    // ע����Ӧ����ص�����
    epoll_ -> setOnResponse(std::bind(&HttpServer::__doResponse, this, std::placeholders::_1));

    // �¼�ѭ��
    while(isRun) {
        int timeMS = TimerQueue_ -> getNextExpireTime();
        // �ȴ��¼�����
        int eventsNum = epoll_ -> wait(timeMS);

        if(eventsNum > 0) {
            // �ַ��¼�������
            epoll_ -> handleEvent(listenFd_, threadPool_, eventsNum);
        }
        TimerQueue_ -> handleExpireTimers();   
    }
    /*�رշ�����ǰ��׼������*/
    std::this_thread::sleep_for(std::chrono::seconds(3));
    exit(1);
}

// ET
void HttpServer::__acceptConnection(){

    while(1) {
        int acceptFd = ::accept4(listenFd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(acceptFd == -1) {
            //�����յ�Interrupt signal
            if(errno == EAGAIN)
                break;
            LOG << "[HttpServer::__acceptConnection] accept : " << strerror(errno) << "\n";
            break;
        }
        // Ϊ�µ������׽��ַ���HttpRequest��Դ
        HttpRequest* request = new HttpRequest(acceptFd);
        TimerQueue_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&HttpServer::__closeConnection, this, request));
        // ע�������׽��ֵ�epoll���ɶ�����Ե��������֤��һʱ��ֻ��һ���̴߳���
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
    // �ͷŸ��׽���ռ�õ�HttpRequest��Դ��������������close(fd)
    delete request;
    request = nullptr;
}

// LTģʽ
void HttpServer::__doRequest(HttpRequest* request){

    TimerQueue_ -> delTimer(request);
    assert(request != nullptr);
    int fd = request -> fd();

    int readErrno;
    int nRead = request -> read(&readErrno);

    // read����0��ʾ�ͻ��˶Ͽ�����
    if(nRead == 0) {
        request -> setNoWorking();
        __closeConnection(request);
        return; 
    }

    // ��EAGAIN���󣬶Ͽ�����
    if(nRead < 0 && (readErrno != EAGAIN)) {
        request -> setNoWorking();
        __closeConnection(request);
        return; 
    }

    // EAGAIN�������ͷ��߳�ʹ��Ȩ���������´οɶ��¼�epoll_ -> mod(...)
    if(nRead < 0 && readErrno == EAGAIN) {
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLONESHOT));
        request -> setNoWorking();
        TimerQueue_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&HttpServer::__closeConnection, this, request));
        return;
    }

    // �������ģ�������Ͽ�����
    if(!request -> parseRequest()) {
        // ����400����
        HttpResponse response(400, "", false);
        request -> appendOutBuffer(response.makeResponse());

        // ���̹ر������ˣ����Ծ���ûд��Ҳֻ��дһ��
        int writeErrno;
        request -> write(&writeErrno);
        request -> setNoWorking();
        __closeConnection(request); 
        return; 
    }

    // �������
    if(request -> parseFinish()) {
        HttpResponse response(200, request -> getPath(), request -> keepAlive(), request->getUsr(), request->getPwd(), request->getTmp(), request->getMethod());
        request -> appendOutBuffer(response.makeResponse());
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
    }
}

// LTģʽ
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

    // ��EAGAIN���󣬶Ͽ�����
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
