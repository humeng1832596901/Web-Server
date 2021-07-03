/*************************************************************************
	> File Name: FileUtil.h
	> Author: 胡孟
	> Mail: 13535324513@163.com
	> Created Time: Tue 23 Mar 2021 04:58:20 PM CST
 ************************************************************************/

#ifndef _FILEUTIL_H
#define _FILEUTIL_H

#include <string>
#include "noncopyable.h"

class AppendFile : noncopyable {
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();
    // append 会向文件写
    void append(const char *logline, const size_t len);
    void flush();

private:
    size_t write(const char *logline, size_t len);
    FILE *fp_;
    char buffer_[64 * 1024];
};

#endif
