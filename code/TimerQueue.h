/*************************************************************************
	> File Name: TimeQueue.h
	> Author: 胡孟
	> Mail: 13535324513@163.com
	> Created Time: Mon 22 Mar 2021 08:44:56 PM CST
 ************************************************************************/

#ifndef _TIMEQUEUE_H
#define _TIMEQUEUE_H

#include "Timer.h"
#include "noncopyable.h"

// 比较函数，用于priority_queue，时间值最小的在队头
struct cmp {
    bool operator()(Timer* a, Timer* b){
        assert(a != nullptr && b != nullptr);
        return (a -> getExpireTime()) > (b -> getExpireTime());
    }
};

class TimerQueue : public noncopyable{
public:
    TimerQueue() 
        : now_(std::chrono::high_resolution_clock::now()) 
    {}

    ~TimerQueue() {}
    void updateTime() { now_ = std::chrono::high_resolution_clock::now(); }
    void addTimer(HttpRequest* request, const int& timeout, const TimeoutCallBack& cb); // timeout单位ms
    void delTimer(HttpRequest* request);
    void handleExpireTimers();
    int getNextExpireTime(); // 返回超时时间(优先队列中最早超时时间和当前时间差)

private:
    std::priority_queue<Timer*, std::vector<Timer*>, cmp> timerQueue_;
    std::chrono::high_resolution_clock::time_point now_;
    std::mutex lock_;
}; // class TimerQueue

#endif
