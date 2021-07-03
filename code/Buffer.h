#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <vector>
#include <string>
#include <algorithm> // copy
#include <iostream>
#include <cassert>

#define INIT_SIZE 1024

class Buffer {
public:
    Buffer()
        : buffer_(INIT_SIZE),
          readerIndex_(0),
          writerIndex_(0)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == INIT_SIZE);
    }
    ~Buffer() {}

    // Ĭ�Ͽ������캯���͸�ֵ��������

    /*�ɶ��ֽ���*/
    size_t readableBytes() const { 
        return writerIndex_ - readerIndex_; 
    }

    /*��д�ֽ���*/
    size_t writableBytes() const { 
        return buffer_.size() - writerIndex_; 
    }

    size_t prependableBytes() const // readerIndex_ǰ��Ŀ��л�������С
    { return readerIndex_; }

    const char* peek() const // ��һ���ɶ�λ��
    { return __begin() + readerIndex_; }

    void retrieve(size_t len) // ȡ��len���ֽ� 
    {
        assert(len <= readableBytes());
        readerIndex_ += len;
    }

    void retrieveUntil(const char* end) // ȡ������ֱ��end
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveAll() // ȡ��buffer��ȫ������
    {
        readerIndex_ = 0;
        writerIndex_ = 0;
    }

    std::string retrieveAsString() // ��string��ʽȡ��ȫ������
    {
        std::string str(peek(), readableBytes());
        retrieveAll();
        return str;
    }

    void append(const std::string& str) // ��������
    { append(str.data(), str.length()); }

    void append(const char* data, size_t len) // ��������
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void append(const void* data, size_t len) // ��������
    { append(static_cast<const char*>(data), len); }

    void append(const Buffer& otherBuff) // ��������������������ӵ���������
    { append(otherBuff.peek(), otherBuff.readableBytes()); }

    void ensureWritableBytes(size_t len) // ȷ�����������㹻�ռ�
    {
        if(writableBytes() < len) {
            __makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char* beginWrite() // ��дcharָ��
    { return __begin() + writerIndex_; }

    const char* beginWrite() const
    { return __begin() + writerIndex_; }

    void hasWritten(size_t len) // д�����ݺ��ƶ�writerIndex_
    { writerIndex_ += len; }

    ssize_t readFd(int fd, int* savedErrno); // ���׽��ֶ���������
    ssize_t writeFd(int fd, int* savedErrno); // ������д���׽���

    const char* findCRLF() const {

        const char CRLF[] = "\r\n";
        const char* crlf = std::search(peek(), beginWrite(), CRLF, CRLF+2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    const char* findCRLF(const char* start) const {

        assert(peek() <= start);
        assert(start <= beginWrite());
        const char CRLF[] = "\r\n";
        const char* crlf = std::search(start, beginWrite(), CRLF, CRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

private:
    char* __begin() // ���ػ�����ͷָ��
    { return &*buffer_.begin(); }

    const char* __begin() const // ���ػ�����ͷָ��
    { return &*buffer_.begin(); }

    void __makeSpace(size_t len) // ȷ�����������㹻�ռ�
    {
        if(writableBytes() + prependableBytes() < len) {
            buffer_.resize(writerIndex_ + len);
        }
        else {
            size_t readable = readableBytes();
            std::copy(__begin() + readerIndex_,
                      __begin() + writerIndex_,
                      __begin());
            readerIndex_ = 0;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }
private:
    /*���Զ�����*/
    std::vector<char> buffer_;
    
    /*���Աʹ��size_t������Ӧ�Ե�����ʧЧ*/
    size_t readerIndex_;
    size_t writerIndex_;
}; // class Buffer

#endif
