#ifndef __TIMER_H__
#define __TIMER_H__

#include <functional>
#include <chrono>
#include <queue>
#include <vector>
#include <iostream>
#include <cassert>
#include <mutex>

using TimeoutCallBack = std::function<void()>;
using MS = std::chrono::milliseconds;

class HttpRequest;

class Timer {
public:
    Timer(const std::chrono::high_resolution_clock::time_point& when, const TimeoutCallBack& cb)
        : expireTime_(when),
          callBack_(cb),
          delete_(false) {}
    ~Timer() {}
    void del() { delete_ = true; }
    bool isDeleted() { return delete_; }
    std::chrono::high_resolution_clock::time_point getExpireTime() const { return expireTime_; }
    void runCallBack() { callBack_(); }

private:
    std::chrono::high_resolution_clock::time_point expireTime_;
    TimeoutCallBack callBack_;
    bool delete_;
}; // class Timer

#endif
