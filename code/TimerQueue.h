/*************************************************************************
	> File Name: TimeQueue.h
	> Author: ����
	> Mail: 13535324513@163.com
	> Created Time: Mon 22 Mar 2021 08:44:56 PM CST
 ************************************************************************/

#ifndef _TIMEQUEUE_H
#define _TIMEQUEUE_H

#include "Timer.h"
#include "noncopyable.h"

// �ȽϺ���������priority_queue��ʱ��ֵ��С���ڶ�ͷ
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
    void addTimer(HttpRequest* request, const int& timeout, const TimeoutCallBack& cb); // timeout��λms
    void delTimer(HttpRequest* request);
    void handleExpireTimers();
    int getNextExpireTime(); // ���س�ʱʱ��(���ȶ��������糬ʱʱ��͵�ǰʱ���)

private:
    std::priority_queue<Timer*, std::vector<Timer*>, cmp> timerQueue_;
    std::chrono::high_resolution_clock::time_point now_;
    std::mutex lock_;
}; // class TimerQueue

#endif
