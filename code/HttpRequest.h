#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include "Buffer.h"
#include <string>
#include <map>
#include <iostream>
#include "noncopyable.h"
#define STATIC_ROOT "../file"

class Timer;

class HttpRequest : public noncopyable{
public:
    enum HttpRequestParseState { // 报文解析状态
        ExpectRequestLine,
        ExpectHeaders,
        ExpectBody,
        GotAll
    };

    enum Method { // HTTP方法
        Invalid, Get, Post, Head, Put, Delete
    };

    enum Version { // HTTP版本
        Unknown, HTTP10, HTTP11
    };

    HttpRequest(int fd);
    ~HttpRequest();

    int fd() { return fd_; } // 返回文件描述符
    int read(int* savedErrno); // 读数据
    int write(int* savedErrno); // 写数据

    void appendOutBuffer(const Buffer& buf) { outBuff_.append(buf); }
    int writableBytes() { return outBuff_.readableBytes(); }

    void setTimer(Timer* timer) { timer_ = timer; }
    Timer* getTimer() { return timer_; }

    void setWorking() { working_ = true; }
    void setNoWorking() { working_ = false; }
    bool isWorking() const { return working_; }

    bool parseRequest(); // 解析Http报文
    bool parseFinish() { return state_ == GotAll; } // 是否解析完一个报文
    void resetParse(); // 重置解析状态
    std::string getPath() const { return path_; }
    std::string getQuery() const { return query_; }
    std::string getUsr() const { return usr_; }
    std::string getPwd() const { return pwd_; }
    std::string getTmp() const {return tmp_; }
    std::string getHeader(const std::string& field) const;
    std::string getMethod() const;
    bool keepAlive() const; // 是否长连接

private:
    // 解析请求行
    bool __parseRequestLine(const char* begin, const char* end);
    // 设置HTTP方法
    bool __setMethod(const char* begin, const char* end);
    // 设置URL路径
    void __setPath(const char* begin, const char* end){
        std::string subPath;
        subPath.assign(begin, end);
        if(subPath[0] == '/'){
            if(subPath=="/")
            subPath = "/index.html";
            else if(subPath.find('.')>=subPath.size())
            subPath += "/index.html";
        }
        /*debug*/
        path_ = STATIC_ROOT + subPath;
    }

    // 设置URL参数
    void __setQuery(const char* begin, const char* end){
        query_.assign(begin, end); 
        int a,b,c,d;
        a = query_.find("=");
        b = query_.find("&");
        c = query_.find("=",b);
        d = query_.find("&",c);
        usr_.assign(query_,a+1,b-a-1);
        pwd_.assign(query_,c+1,d-c-1);
        if(d>=query_.size())
            tmp_ = "";
        else{
            int e = query_.find("=",d);
            tmp_.assign(query_,e+1,query_.size()-e-1);
        }
    }

    // 设置HTTP版本
    void __setVersion(Version version) { version_ = version; }
    // 增加报文头
    void __addHeader(const char* start, const char* colon, const char* end);

private:
    // 网络通信相关
    int fd_; // 文件描述符
    Buffer inBuff_; // 读缓冲区
    Buffer outBuff_; // 写缓冲区
    bool working_; // 若正在工作，则不能被超时事件断开连接

    // 定时器相关
    Timer* timer_;

    // 报文解析相关
    HttpRequestParseState state_; // 报文解析状态
    Method method_; // HTTP方法
    Version version_; // HTTP版本
    std::string path_; // URL路径
    std::string query_; // URL参数
    std::string usr_; //用户名
    std::string pwd_; //密码
    std::string tmp_;
    std::map<std::string, std::string> headers_; // 报文头部
}; // class HttpRequest

#endif
