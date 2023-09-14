#ifndef FILEUTIL
#define FILEUTIL

#include <cstdint>
#include "noncopyable.h"
#include <string>
namespace tinyrpc {

class AppendFile : noncopyable {
public:
    // 在构造函数是打开文件，（a:追加，e，相当于O_CLOEXEC，，在调用exec是会关闭文件描述符）
    explicit AppendFile(std::string filename);
    // 关闭文件
    ~AppendFile();

    // 向缓冲区（flush后会进入文件）写数据
    void append(const char* logline, size_t len);

    void flush();
    // 返回已写入了多少字节
    off_t writtenBytes() const {
        return writtenBytes_;
    }

private:
    size_t write(const char* logline, size_t len);

    FILE* fp_;
    char buffer_[64 * 1024]; // 文件输出缓冲区大小，64kb
    off_t writtenBytes_;     // 已写日志数据的总字节数
};

} // namespace tinyrpc

#endif /* FILEUTIL */
