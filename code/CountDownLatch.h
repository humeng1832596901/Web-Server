#ifndef _COUNTDOWNLATCH_H
#define _COUNTDOWNLATCH_H

#include "noncopyable.h"
#include <mutex>
#include <condition_variable>

// CountDownLatch的主要作用是确保logThread中传进去的func真的启动了以后
// 外层的start才返回
class CountDownLatch : noncopyable {
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    int count_;
};

#endif

