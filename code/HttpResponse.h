#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include <map>
#include <string>

#define CONNECT_TIMEOUT 500 // �ǻ�Ծ����500ms�Ͽ�

class Buffer;

class HttpResponse {
public:
    static const std::map<int, std::string> statusCode2Message;
    static const std::map<std::string, std::string> suffix2Type;

    HttpResponse(int statusCode, std::string path, bool keepAlive, std::string method="GET")
        : statusCode_(statusCode),
          path_(path),
          keepAlive_(keepAlive),
          usr_(""),
          pwd_(""),
          tmp_(""),
          method_(method)
    {}

    HttpResponse(int statusCode, std::string path, bool keepAlive, std::string usr, std::string pwd, std::string tmp, std::string method="GET")
        : statusCode_(statusCode),
          path_(path),
          keepAlive_(keepAlive),
          usr_(usr),
          pwd_(pwd),
          tmp_(tmp),
          method_(method)
    {}

    ~HttpResponse() {}

    Buffer makeResponse();
    void doErrorResponse(Buffer& output, std::string message);
    void doStaticRequest(Buffer& output, long fileSize);

private:
    std::string __getFileType();

private:
    std::map<std::string, std::string> headers_; // ��Ӧ����ͷ��
    int statusCode_; // ��Ӧ״̬��
    std::string path_; // ������Դ·��
    bool keepAlive_; // ������
    std::string usr_; //�û���
    std::string pwd_; //����
    std::string tmp_;
    std::string method_; 
}; // class HttpResponse

#endif
