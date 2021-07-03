#include "LogFile.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "FileUtil.h"

LogFile::LogFile(const std::string& basename, int flushEveryN)
    : basename_(basename),
      flushEveryN_(flushEveryN),
      count_(0),
      mutexPtr_(new std::mutex) {
    file_.reset(new AppendFile(basename));
}

LogFile::~LogFile(){}

void LogFile::append(const char* logline, int len){
    std::unique_lock<std::mutex> lock(*mutexPtr_);
    append_unlocked(logline, len);
}

void LogFile::flush(){
    std::unique_lock<std::mutex> lock(*mutexPtr_);
    file_->flush();
}

void LogFile::append_unlocked(const char* logline, int len){
    file_->append(logline, len);
    ++count_;
    if(count_ >= flushEveryN_){
        count_ = 0;
        file_->flush();
    }
}

