/*************************************************************************
	> File Name: noncopyable.h
	> Author: ºúÃÏ
	> Mail: 13535324513@163.com
	> Created Time: Tue 23 Mar 2021 04:05:05 PM CST
 ************************************************************************/

#ifndef _NONCOPYABLE_H
#define _NONCOPYABLE_H

struct base_token{};

class noncopyable : public base_token{
 protected:
  noncopyable() {}
  ~noncopyable() {}

 private:
  noncopyable(const noncopyable&);
  const noncopyable& operator=(const noncopyable&);
};

#endif
