#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include "noncopyable.h"

class ThreadPool : public noncopyable {
public:
    using JobFunc = std::function<void()>;

    ThreadPool(int numWorkers);
    ~ThreadPool();
    void pushJob(const JobFunc& job);

private:
    std::vector<std::thread> threads_;
    std::mutex lock_;
    std::condition_variable cond_;
    std::queue<JobFunc> jobs_;
    bool stop_;
};

#endif
