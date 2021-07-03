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
