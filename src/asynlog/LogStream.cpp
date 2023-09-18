#include "LogStream.h"
#include <algorithm>
#include <cstdint>
#include <inttypes.h>
#include <limits>
#include <cassert>

namespace tinyrpc {

// 十进制查询表
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");

// 十六进制查询表
const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof(digitsHex) == 17, "wrong number of digitsHex");

// 十进制转str
template <typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    // 处理负数
    if (value < 0) { *p++ = '-'; }
    *p = '\0';
    // 反转
    std::reverse(buf, p);

    return p - buf;
}

// Hex 转 str
size_t convertHex(char buf[], uintptr_t value) {
    uintptr_t i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

// 模板实例化
template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template <int SIZE>
const char* FixedBuffer<SIZE>::debugString() {
    *cur_ = '\0';
    return data_;
}

template <int SIZE>
void FixedBuffer<SIZE>::cookieStart() {
}

template <int SIZE>
void FixedBuffer<SIZE>::cookieEnd() {
}

// 用来检查是否
void LogStream::staticCheck() {
    static_assert(
        kMaxNumericSize - 10 > std::numeric_limits<double>::digits10,
        "kMaxNumericSize is large enough");
    static_assert(
        kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10,
        "kMaxNumericSize is large enough");
    static_assert(
        kMaxNumericSize - 10 > std::numeric_limits<long>::digits10,
        "kMaxNumericSize is large enough");
    static_assert(
        kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10,
        "kMaxNumericSize is large enough");
}

template <typename T>
void LogStream::formatInteger(T v) {
    // 如果剩余空间足够
    if (buffer_.avail() >= kMaxNumericSize) {
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);
    }
}

LogStream& LogStream::operator<<(bool v) {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
}

LogStream& LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
    formatInteger(v);
    return *this;
}

// 指针转换为str
LogStream& LogStream::operator<<(const void* p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer_.avail() >= kMaxNumericSize) {
        char* buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf + 2, v);
        buffer_.add(len + 2);
    }
    return *this;
}

LogStream& LogStream::operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
}

LogStream& LogStream::operator<<(double v) {
    if (buffer_.avail() >= kMaxNumericSize) {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
}

LogStream& LogStream::operator<<(const char* str) {
    if (str) {
        buffer_.append(str, strlen(str));
    } else {
        buffer_.append("(null)", 6);
    }
    return *this;
}

LogStream& LogStream::operator<<(const unsigned char* str) {
    // reinterpret_cast可以将一种指针转换为另一种指针
    return operator<<(reinterpret_cast<const char*>(str));
}

LogStream& LogStream::operator<<(const std::string& v) {
    buffer_.append(v.c_str(), v.size());
    return *this;
}

LogStream& LogStream::operator<<(const pcrecpp::StringPiece& v) {
    buffer_.append(v.data(), v.size());
    return *this;
}
LogStream& LogStream::operator<<(const Buffer& v) {
    *this << v.toStringPiece();
    return *this;
}

LogStream& LogStream::operator<<(const SourceFile& v) {
    buffer_.append(v.data_, v.size_);
    return *this;
}
} // namespace tinyrpc