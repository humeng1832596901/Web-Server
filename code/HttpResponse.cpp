#include "HttpResponse.h"
#include "Buffer.h"
#include "Mydb.h"
#include "Logging.h"

#include <string>
#include <iostream>
#include <cassert>
#include <cstring>

#include <fcntl.h> // open
#include <unistd.h> // close
#include <sys/stat.h> // stat mkdir
#include <sys/types.h> //mkdir
#include <sys/mman.h> // mmap, munmap
#include <cstdlib>

const std::map<int, std::string> HttpResponse::statusCode2Message = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"}
};

const std::map<std::string, std::string> HttpResponse::suffix2Type = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"}
};

Buffer HttpResponse::makeResponse(){
   
    Buffer output;
    
    struct stat sbuf;

    if(method_=="GET" && path_=="../file/register.html"){
        if(usr_.size()==0 || pwd_.size()==0){
            LOG << "用户进入注册界面";
            ::stat(path_.data(), &sbuf);
            doStaticRequest(output,sbuf.st_size);
            return output;
        }
        
        MyDB db;
        db.initDb("127.0.0.1","root","root","mydb");
        if(!db.insertSql(usr_,pwd_)){
            statusCode_ = 403;
            doErrorResponse(output,string("账号已存在"));
            return output;
        }
        std::string new_dir = "../file/" + usr_ + "/";
        mkdir(new_dir.c_str(),0755);
        system(("cp ../file/123/test.py " + new_dir).c_str());
        system(("cp ../file/123/create.sh " + new_dir).c_str());
        system(("./make.sh " + usr_).c_str());
        LOG << "用户注册成功，新账号为" << usr_ << "，新密码为" << pwd_;
        path_ = "../file/index.html";
        ::stat(path_.data(), &sbuf);
        doStaticRequest(output, sbuf.st_size);
        return output;
    }

    if(method_=="GET" && path_=="../file/change.html"){
        if(usr_.size()==0 && pwd_.size()==0){
            LOG << "用户进入修改界面" ;
            ::stat(path_.data(), &sbuf);
            doStaticRequest(output,sbuf.st_size);
            return output;
        }
    
        MyDB db;
        db.initDb("127.0.0.1","root","root","mydb");

        string res;
        if(!db.findSql(usr_,pwd_,res)){
            LOG << "用户" << usr_ << "尝试修改密码错误, " << "初始密码为" << pwd_ << "，新密码为" << tmp_;
            statusCode_ = 403;
            doErrorResponse(output,res);
            return output;
        }

        db.changePwd(usr_,tmp_);
        LOG << "用户" << usr_ << "修改密码成功，" << "初始密码为" << pwd_ << "，新密码为" << tmp_;
        path_ = "../file/index.html";
        usr_ = "";
        pwd_ = "";
        ::stat(path_.data(), &sbuf);
        doStaticRequest(output, sbuf.st_size);
        return output;
    }

    if(statusCode_ == 400){
        doErrorResponse(output, "解析失败");
        return output;
    }
    
    
    /*防止攻击者跳过登录界面 */
    /*
    if(path_!="../file/index.html" && (usr_.size()==0 || pwd_.size()==0)){
        statusCode_ = 403;
        doErrorResponse(output, "返回登录界面继续操作");
        return output;
    }
    */
    
    /*连接mysql*/
    if(path_=="../file/index.html" && usr_.size() != 0 && pwd_.size() != 0){
        MyDB db;
        db.initDb("127.0.0.1","root","root","mydb");
        string res;
        if(!db.findSql(usr_,pwd_,res)){
            statusCode_ = 403;
            doErrorResponse(output, res);
            return output;
        }else{
            /*登录成功导向新页面*/
            path_ = "../file/" + usr_ + "/index.html";
            LOG << "用户 " << usr_ << "||" << pwd_ << " 登陆成功";;
            ::stat(path_.data(), &sbuf);
            doStaticRequest(output, sbuf.st_size);
            return output;
        }
    }
    // 文件找不到错误
    if(::stat(path_.data(), &sbuf) < 0){
        statusCode_ = 404;
        doErrorResponse(output, "查无此文件");
        return output;
    }
    // 权限错误
    if(!(S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode))){
        statusCode_ = 403;
        doErrorResponse(output, "无法操作该文件");
        return output;
    }

    // 处理静态文件请求
    doStaticRequest(output, sbuf.st_size);
    return output;
}

//  还要填入哪些报文头部选项
void HttpResponse::doStaticRequest(Buffer& output, long fileSize){

    assert(fileSize >= 0);

    auto itr = statusCode2Message.find(statusCode_);
    if(itr == statusCode2Message.end()){
        statusCode_ = 400;
        doErrorResponse(output, "未知状态码");
        return;
    }

    // 响应行
    output.append("HTTP/1.1 " + std::to_string(statusCode_) + " " + itr -> second + "\r\n");
    // 报文头
    if(keepAlive_) {
        output.append("Connection: Keep-Alive\r\n");
        output.append("Keep-Alive: timeout=" + std::to_string(CONNECT_TIMEOUT) + "\r\n");
    } else {
        output.append("Connection: close\r\n");
    }
    output.append("Content-type: " + __getFileType() + "\r\n");
    output.append("Content-length: " + std::to_string(fileSize) + "\r\n");
    // 添加头部Last-Modified: ?
    output.append("Server: My Server\r\n");
    output.append("\r\n");

    // 报文体
    int srcFd = ::open(path_.data(), O_RDONLY, 0);
    // 存储映射IO
    void* mmapRet = ::mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, srcFd, 0);
    ::close(srcFd);
    if(mmapRet == (void*) -1){
        munmap(mmapRet, fileSize);
        output.retrieveAll();
        statusCode_ = 404;
        doErrorResponse(output, "查无此文件");
        return;
    }
    char* srcAddr = static_cast<char*>(mmapRet);
    output.append(srcAddr, fileSize);

    munmap(srcAddr, fileSize);
}

std::string HttpResponse::__getFileType(){

    int idx = path_.find_last_of('.');
    std::string suffix;
    // 找不到文件后缀，默认纯文本
    if(idx == std::string::npos){
        return "text/plain";
    }
        
    suffix = path_.substr(idx);
    auto itr = suffix2Type.find(suffix);
    // 未知文件后缀，默认纯文本
    if(itr == suffix2Type.end()){
        return "text/plain";
    }   
    return itr -> second;
}

//  还要填入哪些报文头部选项
void HttpResponse::doErrorResponse(Buffer& output, std::string message){
    
    std::string body;

    auto itr = statusCode2Message.find(statusCode_);
    if(itr == statusCode2Message.end()){
        return;
    }

    body += "<html><meta charset=\"UTF-8\">\n<title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += std::to_string(statusCode_) + " : " + itr -> second + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>My Server</em></body></html>";

    // 响应行
    output.append("HTTP/1.1 " + std::to_string(statusCode_) + " " + itr -> second + "\r\n");
    // 报文头
    output.append("Server: My Server\r\n");
    output.append("Content-type: text/html\r\n");
    output.append("Connection: close\r\n");
    output.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    // 报文体
    output.append(body);
}
