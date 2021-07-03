#ifndef __EPOLL_H__
#define __EPOLL_H__

#include <functional> // function
#include <vector> // vector
#include <memory> // shared_ptr
#include <sys/epoll.h> // epoll_event

#define MAXEVENTS 1024

class HttpRequest;
class ThreadPool;

class Epoll {
public:
    using NewConnectionCallback = std::function<void()>;
    using CloseConnectionCallback = std::function<void(HttpRequest*)>;
    using HandleRequestCallback = std::function<void(HttpRequest*)>;
    using HandleResponseCallback = std::function<void(HttpRequest*)>;

    Epoll();
    ~Epoll();
    int add(int fd, HttpRequest* request, int events); // ע����������
    int mod(int fd, HttpRequest* request, int events); // �޸�������״̬
    int del(int fd, HttpRequest* request, int events); // ��epoll��ɾ��������
    int wait(int timeoutMs); // �ȴ��¼�����, ���ػ�Ծ����������
    void handleEvent(int listenFd, std::shared_ptr<ThreadPool>& threadPool, int eventsNum); // �����¼�������
    void setOnConnection(const NewConnectionCallback& cb) { onConnection_ = cb; } // ���������ӻص�����
    void setOnCloseConnection(const CloseConnectionCallback& cb) { onCloseConnection_ = cb; } // ���ùر����ӻص�����
    void setOnRequest(const HandleRequestCallback& cb) { onRequest_ = cb; } // ���ô�������ص�����
    void setOnResponse(const HandleResponseCallback& cb) { onResponse_ = cb; } // ������Ӧ����ص�����

private: 
    using EventList = std::vector<struct epoll_event>;
    
    int epollFd_;
    EventList events_;
    NewConnectionCallback onConnection_;
    CloseConnectionCallback onCloseConnection_;
    HandleRequestCallback onRequest_;
    HandleResponseCallback onResponse_;
}; // class Epoll

#endif
