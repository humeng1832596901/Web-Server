/*************************************************************************
	> File Name: CurrentThread.h
	> Author: ºúÃÏ
	> Mail: 13535324513@163.com
	> Created Time: Fri 26 Mar 2021 08:28:58 PM CST
 ************************************************************************/

#ifndef _CURRENTTHREAD_H
#define _CURRENTTHREAD_H

#include <stdint.h>

namespace CurrentThread{
    // internal
    extern __thread int t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char* t_threadName;
    void cacheTid();
    inline int tid(){
        if (__builtin_expect(t_cachedTid == 0, 0)){
            cacheTid();
        }
        return t_cachedTid;
    }
    
    //for logging
    inline const char* tidString(){
        return t_tidString;
    }

    inline int tidStringLength(){
        return t_tidStringLength;
    }

    inline const char* name() { return t_threadName; }
}

#endif
