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
    enum HttpRequestParseState { // ���Ľ���״̬
        ExpectRequestLine,
        ExpectHeaders,
        ExpectBody,
        GotAll
    };

    enum Method { // HTTP����
        Invalid, Get, Post, Head, Put, Delete
    };

    enum Version { // HTTP�汾
        Unknown, HTTP10, HTTP11
    };

    HttpRequest(int fd);
    ~HttpRequest();

    int fd() { return fd_; } // �����ļ�������
    int read(int* savedErrno); // ������
    int write(int* savedErrno); // д����

    void appendOutBuffer(const Buffer& buf) { outBuff_.append(buf); }
    int writableBytes() { return outBuff_.readableBytes(); }

    void setTimer(Timer* timer) { timer_ = timer; }
    Timer* getTimer() { return timer_; }

    void setWorking() { working_ = true; }
    void setNoWorking() { working_ = false; }
    bool isWorking() const { return working_; }

    bool parseRequest(); // ����Http����
    bool parseFinish() { return state_ == GotAll; } // �Ƿ������һ������
    void resetParse(); // ���ý���״̬
    std::string getPath() const { return path_; }
    std::string getQuery() const { return query_; }
    std::string getUsr() const { return usr_; }
    std::string getPwd() const { return pwd_; }
    std::string getTmp() const {return tmp_; }
    std::string getHeader(const std::string& field) const;
    std::string getMethod() const;
    bool keepAlive() const; // �Ƿ�����

private:
    // ����������
    bool __parseRequestLine(const char* begin, const char* end);
    // ����HTTP����
    bool __setMethod(const char* begin, const char* end);
    // ����URL·��
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

    // ����URL����
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

    // ����HTTP�汾
    void __setVersion(Version version) { version_ = version; }
    // ���ӱ���ͷ
    void __addHeader(const char* start, const char* colon, const char* end);

private:
    // ����ͨ�����
    int fd_; // �ļ�������
    Buffer inBuff_; // ��������
    Buffer outBuff_; // д������
    bool working_; // �����ڹ��������ܱ���ʱ�¼��Ͽ�����

    // ��ʱ�����
    Timer* timer_;

    // ���Ľ������
    HttpRequestParseState state_; // ���Ľ���״̬
    Method method_; // HTTP����
    Version version_; // HTTP�汾
    std::string path_; // URL·��
    std::string query_; // URL����
    std::string usr_; //�û���
    std::string pwd_; //����
    std::string tmp_;
    std::map<std::string, std::string> headers_; // ����ͷ��
}; // class HttpRequest

#endif
