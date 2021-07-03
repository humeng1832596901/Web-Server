#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <memory> // unique_ptr
#include <mutex>
#include "noncopyable.h"

#define TIMEOUTMS -1 // epoll_wait超时时间，-1表示不设超时
#define CONNECT_TIMEOUT 500 // 连接默认超时时间
#define NUM_WORKERS 6 // 线程池大小

class HttpRequest;
class Epoll;
class ThreadPool;
class TimerQueue;

class HttpServer : public noncopyable{
public:
    HttpServer(int port, int numThread);
    ~HttpServer();
    void run(); // 启动HTTP服务器
    void stop();
    
private:
    void __acceptConnection(); // 接受新连接
    void __closeConnection(HttpRequest* request); // 关闭连接
    void __doRequest(HttpRequest* request); // 处理HTTP请求报文，这个函数由线程池调用
    void __doResponse(HttpRequest* request);

private:
    using ListenRequestPtr = std::unique_ptr<HttpRequest>;
    using EpollPtr = std::unique_ptr<Epoll>;
    using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
    using TimerQueuePtr = std::unique_ptr<TimerQueue>;

    bool isRun;
    int port_; // 监听端口
    int listenFd_; // 监听套接字
    ListenRequestPtr listenRequest_; // 监听套接字的HttpRequest实例
    EpollPtr epoll_; // epoll实例
    ThreadPoolPtr threadPool_; // 线程池
    TimerQueuePtr TimerQueue_; // 定时器队列
}; // class HttpServer

#endif
