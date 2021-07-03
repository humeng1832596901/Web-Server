/*************************************************************************
	> File Name: Thread.h
	> Author: ºúÃÏ
	> Mail: 13535324513@163.com
	> Created Time: Tue 23 Mar 2021 06:46:14 PM CST
 ************************************************************************/

#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
//#include <thread>
#include <functional>
#include <memory>
#include <string>
#include "CountDownLatch.h"
#include "noncopyable.h"

class Thread : noncopyable {
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(const ThreadFunc&, const std::string& name = std::string());
    ~Thread();
    void start();
    int join();
    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }

private:
    void setDefaultName();
    bool started_;
    bool joined_;
    //std::thread t_;
    //unsigned long 
    pthread_t pthreadId_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;
};

#endif
