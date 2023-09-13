#ifndef LOGSTREAM
#define LOGSTREAM

/*重载 >> ,并且定义迭代器
type -> string
*/

#include "SoruceFile.h"
#include "Types.h"
#include "noncopyable.h"
#include "FixedBuffer.h"
#include <cstddef>
#include <string>
#include <pcre_stringpiece.h>
#include <cassert>
namespace tinyrpc {

class LogStream : noncopyable {
    typedef LogStream self;

public:
    typedef FixedBuffer<kSmallBuffer> Buffer;
    // 对<<(输入符号的重载)
    self& operator<<(bool v);
    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);
    self& operator<<(const void*);
    self& operator<<(float v);
    self& operator<<(double);
    self& operator<<(char v);
    self& operator<<(const char* str);
    self& operator<<(const unsigned char* str);
    self& operator<<(const std::string& v);
    self& operator<<(const pcrecpp::StringPiece& v);
    self& operator<<(const Buffer& v);
    self& operator<<(const SourceFile& v);

    void append(const char* data, int len) {
        buffer_.append(data, len);
    }
    const Buffer& buffer() const {
        return buffer_;
    }
    void resetBuffer() {
        buffer_.reset();
    }

private:
    void staticCheck();

    // 将整形转换为字符串的模板函数
    template <typename T>
    void formatInteger(T);

    Buffer buffer_;

    // 数字的最大位数
    static const int kMaxNumericSize = 48;
};

} // namespace tinyrpc
#endif /* LOGSTREAM */
