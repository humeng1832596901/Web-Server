/*************************************************************************
	> File Name: noncopyable.h
	> Author: ºúÃÏ
	> Mail: 13535324513@163.com
	> Created Time: Mon 22 Mar 2021 10:37:28 PM CST
 ************************************************************************/

#include "ThreadPool.h"
#include <iostream>
#include <cassert>

ThreadPool::ThreadPool(int numWorkers )
    : stop_(false)
    {
        numWorkers = numWorkers <= 0 ? 1 : numWorkers;
        for(int i = 0; i < numWorkers; ++i)
            threads_.emplace_back([this]() {
            while(1) {
            JobFunc func;
            /*¼õÐ¡ËøµÄÁ£¶È*/
            {
                std::unique_lock<std::mutex> lock(lock_);    
                while(!stop_ && jobs_.empty())
                    cond_.wait(lock);
                if(jobs_.empty() && stop_) {
                    return;
                }
                func = jobs_.front();
                jobs_.pop();
            }
            if(func){
                func();
            } 
        }
    });
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(lock_);
        stop_ = true;
    } 
    cond_.notify_all();
    for(auto& thread: threads_)
        thread.join();
}

void ThreadPool::pushJob(const JobFunc& job)
{
    {
        std::unique_lock<std::mutex> lock(lock_);
        jobs_.push(job);
    }
    cond_.notify_one();
}
