#ifndef __UTILS_H__ 
#define __UTILS_H__ 

#define LISTENQ 1024 // �������г���,����ϵͳĬ��ֵΪSOMAXCONN


namespace utils {
    int createListenFd(int port); // ��������������
    int setNonBlocking(int fd); // ���÷�����ģʽ
}

#endif
