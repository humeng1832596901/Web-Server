/*************************************************************************
	> File Name: Logging.h
	> Author: ����
	> Mail: 13535324513@163.com
	> Created Time: Tue 23 Mar 2021 04:55:03 PM CST
 ************************************************************************/

#ifndef _LOGGING_H
#define _LOGGING_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "LogStream.h"

class AsyncLogging;

class Logger {
public:
    Logger(const char *fileName, int line);
    ~Logger();
    LogStream &stream() { return impl_.stream_; }

    static void setLogFileName(std::string fileName) { logFileName_ = fileName; }
    static std::string getLogFileName() { return logFileName_; }

private:
    class Impl {
        public:
        Impl(const char *fileName, int line);
        void formatTime();

        LogStream stream_;
        int line_;
        std::string basename_;
    };
    Impl impl_;
    static std::string logFileName_;
};

#define LOG Logger(__FILE__, __LINE__).stream()

#endif
