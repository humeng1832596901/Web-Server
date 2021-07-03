#include "ListenUtil.h"

#include <iostream>
#include <cstring> // bzero

#include <stdio.h> // perror
#include <unistd.h> // fcntl, close
#include <fcntl.h> // fcntl
#include <sys/socket.h> // socket, setsockopt, bind, listen
#include <arpa/inet.h> // htonl, htons
#include <openssl/ssl.h>
#include <openssl/err.h>

int utils::createListenFd(int port){

    // ����Ƿ��˿�
    port = ((port <= 1024) || (port >= 65535)) ? 6666 : port;

    // �����׽��֣�IPv4��TCP����������
    int listenFd = 0;
    if((listenFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        printf("[Utils::createListenFd]fd = %d socket : %s\n", listenFd, strerror(errno));
        return -1;
    }

    // ���ö˿ڸ���
    int optval = 1;
    if(::setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) == -1){
        printf("[Utils::createListenFd]fd = %d setsockopt : %s\n", listenFd, strerror(errno));
        return -1;
    }
    
    // ��IP�Ͷ˿�
    struct sockaddr_in serverAddr;
    ::bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons((unsigned short)port);
    if(::bind(listenFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1){
        printf("[Utils::createListenFd]fd = %d bind : %s\n", listenFd, strerror(errno));
        return -1;
    }

    // ��ʼ����������������ΪLISTENQ
    if(::listen(listenFd, LISTENQ) == -1){
        printf("[Utils::createListenFd]fd = %d listen : %s\n", listenFd, strerror(errno));
        return -1;
    }

    // �ر���Ч����������
    if(listenFd == -1){
        ::close(listenFd);
        return -1;
    }

    return listenFd;
}

int utils::setNonBlocking(int fd){

    // ��ȡ�׽���ѡ��
    int flag = ::fcntl(fd, F_GETFL, 0);
    if(flag == -1){
        printf("[Utils::setNonBlocking]fd = %d fcntl : %s\n", fd, strerror(errno));
        return -1;
    }
    // ���÷�����
    flag |= O_NONBLOCK;
    if(::fcntl(fd, F_SETFL, flag) == -1){
        printf("[Utils::setNonBlocking]fd = %d fcntl : %s\n", fd, strerror(errno));
        return -1;
    }

    return 0;
}

