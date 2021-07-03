#include "HttpRequest.h"
#include <iostream>
#include <cassert>
#include <unistd.h>

HttpRequest::HttpRequest(int fd)
    : fd_(fd),
      working_(false),
      timer_(nullptr),
      state_(ExpectRequestLine),
      method_(Invalid),
      version_(Unknown)
{
    assert(fd_ >= 0);
}

HttpRequest::~HttpRequest()
{
    close(fd_);
}

int HttpRequest::read(int* savedErrno){
    return inBuff_.readFd(fd_, savedErrno);
}

int HttpRequest::write(int* savedErrno){
    return outBuff_.writeFd(fd_, savedErrno);
}

bool HttpRequest::parseRequest(){
    bool ok = true;
    bool hasMore = true;

    while(hasMore) {
        if(state_ == ExpectRequestLine){
            // ����������
            const char* crlf = inBuff_.findCRLF();
            if(crlf){
                ok = __parseRequestLine(inBuff_.peek(), crlf);
                if(ok){
                    inBuff_.retrieveUntil(crlf + 2);
                    state_ = ExpectHeaders;
                }else{
                    hasMore = false;
                }
            }else{
                hasMore = false;
            }
        }else if(state_ == ExpectHeaders){
            // ������ͷ
            const char* crlf = inBuff_.findCRLF();
            if(crlf){
                const char* colon = std::find(inBuff_.peek(), crlf, ':');
                if(colon != crlf){
                    __addHeader(inBuff_.peek(), colon, crlf);
                } else {
                    if(this->method_==Get){
                        state_ = GotAll;
                        hasMore = false;

                    }else if(this->method_==Post){
                        state_ = ExpectBody;
                        hasMore = true;
                    }
                }
                inBuff_.retrieveUntil(crlf + 2);
                //if(this->method_==Post){
                //    crlf = inBuff_.findCRLF();
                //    inBuff_.retrieveUntil(crlf + 2);
                //}
            }else{
                hasMore = false;
            } 
        }else if(state_ == ExpectBody){
            std::cout << "hello" << std::endl;
            /*
            const char* colon = std::find(inBuff_.peek(), crlf, 'u');
            if(colon != crlf){
                std::cout << "yes" << std::endl;
                while(colon != crlf){
                    std::cout << *colon;
                    colon++;
                }
            }
            */
            state_ = GotAll;
            hasMore = false;
            // �������� 
        }
    }
    
    return ok;
}

bool HttpRequest::__parseRequestLine(const char* begin, const char* end){
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if(space != end && __setMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if(space != end) {
            const char* question = std::find(start, space, '?');
            if(question != space) {
                __setPath(start, question);
                __setQuery(question, space);
            }else{
                __setPath(start, space);
            }
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if(succeed){
                if(*(end - 1) == '1')
                    __setVersion(HTTP11);
                else if(*(end - 1) == '0')
                    __setVersion(HTTP10);
                else
                    succeed = false;
            } 
        }
    }
    return succeed;
}

bool HttpRequest::__setMethod(const char* start, const char* end){
    std::string m(start, end);
    if(m == "GET")
        method_ = Get;
    else if(m == "POST")
        method_ = Post;
    else if(m == "HEAD")
        method_ = Head;
    else if(m == "PUT")
        method_ = Put;
    else if(m == "DELETE")
        method_ = Delete;
    else
        method_ = Invalid;

    return method_ != Invalid;
}

void HttpRequest::__addHeader(const char* start, const char* colon, const char* end){
    
    std::string field(start, colon);
    ++colon;
    while(colon < end && *colon == ' ')
        ++colon;
    std::string value(colon, end);
    while(!value.empty() && value[value.size() - 1] == ' ')
        value.resize(value.size() - 1);

    headers_[field] = value;
}

std::string HttpRequest::getMethod() const {
    
    std::string res;
    if(method_ == Get)
        res = "GET";
    else if(method_ == Post)
        res = "POST";
    else if(method_ == Head)
        res = "HEAD";
    else if(method_ == Put)
        res = "Put";
    else if(method_ == Delete)
        res = "DELETE";
    
    return res;
}

std::string HttpRequest::getHeader(const std::string& field) const {
    
    std::string res;
    auto itr = headers_.find(field);
    if(itr != headers_.end())
        res = itr -> second;
    return res;
}

bool HttpRequest::keepAlive() const {
    
    std::string connection = getHeader("Connection");
    bool res = connection == "Keep-Alive" || 
               (version_ == HTTP11 && connection != "close");

    return res;
}

void HttpRequest::resetParse(){
    state_ = ExpectRequestLine; // ���Ľ���״̬
    method_ = Invalid; // HTTP����
    version_ = Unknown; // HTTP�汾
    path_ = ""; // URL·��
    query_ = ""; // URL����
    headers_.clear(); // ����ͷ��
}
