/*************************************************************************
	> File Name: AsyncLogging.h
	> Author: ºúÃÏ
	> Mail: 13535324513@163.com
	> Created Time: Tue 23 Mar 2021 04:55:51 PM CST
 ************************************************************************/

#ifndef _ASYNCLOGGING_H
#define _ASYNCLOGGING_H

#include <functional>
#include <string>
#include <vector>
#include "CountDownLatch.h"
#include "LogStream.h"
#include <mutex>
#include <condition_variable>
#include "LogThread.h"
#include "noncopyable.h"


class AsyncLogging : noncopyable {
public:
    AsyncLogging(const std::string basename, int flushInterval = 2);
    ~AsyncLogging() {
        if (running_) stop();
    }
    void append(const char* logline, int len);

    void start() {
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    void stop() {
        running_ = false;
        cond_.notify_one();
        thread_.join();
    }

private:
    void threadFunc();
    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferPtr = std::shared_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;
    const int flushInterval_;
    bool running_;
    std::string basename_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
    CountDownLatch latch_;
};

#endif
