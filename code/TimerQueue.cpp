/*************************************************************************
	> File Name: TimeQueue.cpp
	> Author: ����
	> Mail: 13535324513@163.com
	> Created Time: Mon 22 Mar 2021 08:53:14 PM CST
 ************************************************************************/

#include "TimerQueue.h"
#include "HttpRequest.h"
#include <cassert>

void TimerQueue::addTimer(HttpRequest* request, 
                     const int& timeout, 
                     const TimeoutCallBack& cb){
    std::unique_lock<std::mutex> lock(lock_);
    assert(request != nullptr);

    updateTime();
    Timer* timer = new Timer(now_ + MS(timeout), cb);
    timerQueue_.push(timer);

    // ��ͬһ��request������������addTimer����Ҫ��ǰһ����ʱ��ɾ��
    if(request -> getTimer() != nullptr)
        delTimer(request);

    request -> setTimer(timer);
}

// �����������������û���̰߳�ȫ����
// ����������������Ϊ���������������������handleExpireTimers -> runCallBack -> __closeConnection -> delTimer
void TimerQueue::delTimer(HttpRequest* request){

    // std::unique_lock<std::mutex> lock(lock_);
    assert(request != nullptr);

    Timer* timer = request -> getTimer();
    if(timer == nullptr)
        return;

    // �������д��delete timeNode����ʹpriority_queue��Ķ�Ӧָ���ɴ���ָ��
    // ��ȷ�ķ����Ƕ���ɾ��
    timer -> del();
    // ��ֹrequest -> getTimer()���ʵ�����ָ��
    request -> setTimer(nullptr);
}

void TimerQueue::handleExpireTimers(){

    std::unique_lock<std::mutex> lock(lock_);
    updateTime();
    while(!timerQueue_.empty()) {
        Timer* timer = timerQueue_.top();
        assert(timer != nullptr);
        // ��ʱ����ɾ��
        if(timer -> isDeleted()) {
            timerQueue_.pop();
            delete timer;
            continue;
        }
        // ���ȶ���ͷ���Ķ�ʱ��Ҳû�г�ʱ��return
        if(std::chrono::duration_cast<MS>(timer -> getExpireTime() - now_).count() > 0) {
            return;
        }
        // ��ʱ
        timer -> runCallBack();
        timerQueue_.pop();
        delete timer;
    }
}

int TimerQueue::getNextExpireTime(){

    std::unique_lock<std::mutex> lock(lock_);
    updateTime();
    int res = -1;
    while(!timerQueue_.empty()) {
        Timer* timer = timerQueue_.top();
        if(timer -> isDeleted()) {
            timerQueue_.pop();
            delete timer;
            continue;
        }
        res = std::chrono::duration_cast<MS>(timer -> getExpireTime() - now_).count();
        res = (res < 0) ? 0 : res;
        break;
    }
    return res;
}

