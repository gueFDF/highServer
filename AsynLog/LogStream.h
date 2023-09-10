#ifndef LOGSTREAM
#define LOGSTREAM
#include "Types.h"
#include "noncopyable.h"
#include <cstddef>
#include <string>
#endif /* LOGSTREAM */
template <int SIZE> class FixedBuffer : noncopyable {
public:
  FixedBuffer() : cur_(data_) { setCookie(cookieStart()); };
  ~FixedBuffer() { setCookie(cookieEnd()); }

  // 将数据写入缓冲区
  void append(const char *buf, size_t len) {
    if (avail() > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  // 返回缓冲区首地址
  const char *data() const { return data_; }
  // 返回缓冲区长度
  int length() const { return static_cast<int>(cur_ - data_); }
  // 返回缓冲区当前位置
  char *current() { return cur_; }
  // 返回空闲缓冲区长度
  int avail() const { return static_cast<int>(end() - cur_); }
  // 增加已写入数据的长队,(cur指针往后移)
  void add(size_t len) { cur_ += len; }
  // 重置缓冲区(将cur_重新指向开头)
  void reset() { cur_ = data_; }
  // 将缓冲区全置空
  void bzero() { memZero(data_, sizeof data_); }

  // for used by GDB
  const char *debugString();
  void setCookie(void (*cookie)()) { cookie_ = cookie; }
  // for used by unit test
  // 供单元测试使用
  std::string toString() const { return string(data_, length()); }

private:
  const char *end() const { return data_ + sizeof(data_); }

  static void cookieStart();
  static void cookieEnd();
  void (*cookie_)();
  char data_[SIZE];
  char *cur_;
};