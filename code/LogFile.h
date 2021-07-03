/*************************************************************************
	> File Name: LogFile.h
	> Author: ����
	> Mail: 13535324513@163.com
	> Created Time: Tue 23 Mar 2021 04:57:39 PM CST
 ************************************************************************/

#ifndef _LOGFILE_H
#define _LOGFILE_H

#include <memory>
#include <string>
#include <mutex>
#include "FileUtil.h"
#include "noncopyable.h"


// �ṩ�Զ��鵵����
class LogFile : noncopyable {
public:
    // ÿ��append flushEveryN�Σ�flushһ�£������ļ�д��ֻ�������ļ�Ҳ�Ǵ���������
    LogFile(const std::string& basename, int flushEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();

private:
    void append_unlocked(const char* logline, int len);

    const std::string basename_;
    const int flushEveryN_;

    int count_;
    std::unique_ptr<std::mutex> mutexPtr_;
    std::unique_ptr<AppendFile> file_;
};

#endif
