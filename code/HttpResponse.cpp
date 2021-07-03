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
            LOG << "�û�����ע�����";
            ::stat(path_.data(), &sbuf);
            doStaticRequest(output,sbuf.st_size);
            return output;
        }
        
        MyDB db;
        db.initDb("127.0.0.1","root","root","mydb");
        if(!db.insertSql(usr_,pwd_)){
            statusCode_ = 403;
            doErrorResponse(output,string("�˺��Ѵ���"));
            return output;
        }
        std::string new_dir = "../file/" + usr_ + "/";
        mkdir(new_dir.c_str(),0755);
        system(("cp ../file/123/test.py " + new_dir).c_str());
        system(("cp ../file/123/create.sh " + new_dir).c_str());
        system(("./make.sh " + usr_).c_str());
        LOG << "�û�ע��ɹ������˺�Ϊ" << usr_ << "��������Ϊ" << pwd_;
        path_ = "../file/index.html";
        ::stat(path_.data(), &sbuf);
        doStaticRequest(output, sbuf.st_size);
        return output;
    }

    if(method_=="GET" && path_=="../file/change.html"){
        if(usr_.size()==0 && pwd_.size()==0){
            LOG << "�û������޸Ľ���" ;
            ::stat(path_.data(), &sbuf);
            doStaticRequest(output,sbuf.st_size);
            return output;
        }
    
        MyDB db;
        db.initDb("127.0.0.1","root","root","mydb");

        string res;
        if(!db.findSql(usr_,pwd_,res)){
            LOG << "�û�" << usr_ << "�����޸��������, " << "��ʼ����Ϊ" << pwd_ << "��������Ϊ" << tmp_;
            statusCode_ = 403;
            doErrorResponse(output,res);
            return output;
        }

        db.changePwd(usr_,tmp_);
        LOG << "�û�" << usr_ << "�޸�����ɹ���" << "��ʼ����Ϊ" << pwd_ << "��������Ϊ" << tmp_;
        path_ = "../file/index.html";
        usr_ = "";
        pwd_ = "";
        ::stat(path_.data(), &sbuf);
        doStaticRequest(output, sbuf.st_size);
        return output;
    }

    if(statusCode_ == 400){
        doErrorResponse(output, "����ʧ��");
        return output;
    }
    
    
    /*��ֹ������������¼���� */
    /*
    if(path_!="../file/index.html" && (usr_.size()==0 || pwd_.size()==0)){
        statusCode_ = 403;
        doErrorResponse(output, "���ص�¼�����������");
        return output;
    }
    */
    
    /*����mysql*/
    if(path_=="../file/index.html" && usr_.size() != 0 && pwd_.size() != 0){
        MyDB db;
        db.initDb("127.0.0.1","root","root","mydb");
        string res;
        if(!db.findSql(usr_,pwd_,res)){
            statusCode_ = 403;
            doErrorResponse(output, res);
            return output;
        }else{
            /*��¼�ɹ�������ҳ��*/
            path_ = "../file/" + usr_ + "/index.html";
            LOG << "�û� " << usr_ << "||" << pwd_ << " ��½�ɹ�";;
            ::stat(path_.data(), &sbuf);
            doStaticRequest(output, sbuf.st_size);
            return output;
        }
    }
    // �ļ��Ҳ�������
    if(::stat(path_.data(), &sbuf) < 0){
        statusCode_ = 404;
        doErrorResponse(output, "���޴��ļ�");
        return output;
    }
    // Ȩ�޴���
    if(!(S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode))){
        statusCode_ = 403;
        doErrorResponse(output, "�޷��������ļ�");
        return output;
    }

    // ����̬�ļ�����
    doStaticRequest(output, sbuf.st_size);
    return output;
}

//  ��Ҫ������Щ����ͷ��ѡ��
void HttpResponse::doStaticRequest(Buffer& output, long fileSize){

    assert(fileSize >= 0);

    auto itr = statusCode2Message.find(statusCode_);
    if(itr == statusCode2Message.end()){
        statusCode_ = 400;
        doErrorResponse(output, "δ֪״̬��");
        return;
    }

    // ��Ӧ��
    output.append("HTTP/1.1 " + std::to_string(statusCode_) + " " + itr -> second + "\r\n");
    // ����ͷ
    if(keepAlive_) {
        output.append("Connection: Keep-Alive\r\n");
        output.append("Keep-Alive: timeout=" + std::to_string(CONNECT_TIMEOUT) + "\r\n");
    } else {
        output.append("Connection: close\r\n");
    }
    output.append("Content-type: " + __getFileType() + "\r\n");
    output.append("Content-length: " + std::to_string(fileSize) + "\r\n");
    // ���ͷ��Last-Modified: ?
    output.append("Server: My Server\r\n");
    output.append("\r\n");

    // ������
    int srcFd = ::open(path_.data(), O_RDONLY, 0);
    // �洢ӳ��IO
    void* mmapRet = ::mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, srcFd, 0);
    ::close(srcFd);
    if(mmapRet == (void*) -1){
        munmap(mmapRet, fileSize);
        output.retrieveAll();
        statusCode_ = 404;
        doErrorResponse(output, "���޴��ļ�");
        return;
    }
    char* srcAddr = static_cast<char*>(mmapRet);
    output.append(srcAddr, fileSize);

    munmap(srcAddr, fileSize);
}

std::string HttpResponse::__getFileType(){

    int idx = path_.find_last_of('.');
    std::string suffix;
    // �Ҳ����ļ���׺��Ĭ�ϴ��ı�
    if(idx == std::string::npos){
        return "text/plain";
    }
        
    suffix = path_.substr(idx);
    auto itr = suffix2Type.find(suffix);
    // δ֪�ļ���׺��Ĭ�ϴ��ı�
    if(itr == suffix2Type.end()){
        return "text/plain";
    }   
    return itr -> second;
}

//  ��Ҫ������Щ����ͷ��ѡ��
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

    // ��Ӧ��
    output.append("HTTP/1.1 " + std::to_string(statusCode_) + " " + itr -> second + "\r\n");
    // ����ͷ
    output.append("Server: My Server\r\n");
    output.append("Content-type: text/html\r\n");
    output.append("Connection: close\r\n");
    output.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    // ������
    output.append(body);
}
