#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <memory> // unique_ptr
#include <mutex>
#include "noncopyable.h"

#define TIMEOUTMS -1 // epoll_wait��ʱʱ�䣬-1��ʾ���賬ʱ
#define CONNECT_TIMEOUT 500 // ����Ĭ�ϳ�ʱʱ��
#define NUM_WORKERS 6 // �̳߳ش�С

class HttpRequest;
class Epoll;
class ThreadPool;
class TimerQueue;

class HttpServer : public noncopyable{
public:
    HttpServer(int port, int numThread);
    ~HttpServer();
    void run(); // ����HTTP������
    void stop();
    
private:
    void __acceptConnection(); // ����������
    void __closeConnection(HttpRequest* request); // �ر�����
    void __doRequest(HttpRequest* request); // ����HTTP�����ģ�����������̳߳ص���
    void __doResponse(HttpRequest* request);

private:
    using ListenRequestPtr = std::unique_ptr<HttpRequest>;
    using EpollPtr = std::unique_ptr<Epoll>;
    using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
    using TimerQueuePtr = std::unique_ptr<TimerQueue>;

    bool isRun;
    int port_; // �����˿�
    int listenFd_; // �����׽���
    ListenRequestPtr listenRequest_; // �����׽��ֵ�HttpRequestʵ��
    EpollPtr epoll_; // epollʵ��
    ThreadPoolPtr threadPool_; // �̳߳�
    TimerQueuePtr TimerQueue_; // ��ʱ������
}; // class HttpServer

#endif
