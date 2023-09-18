#include "AppendFile.h"
#include <string>
#include "Error_no.h"
#include <cassert>
#include <iostream>
namespace tinyrpc {

AppendFile::AppendFile(std::string filename) :
    fp_(::fopen(filename.c_str(), "ae")), // 'e' for O_CLOEXEC
    writtenBytes_(0) {
    assert(fp_);
    // 将缓冲区设置为本地的bufer(使用较大的内核缓冲区,一次性可以flush更多的数据)
    ::setbuffer(fp_, buffer_, sizeof buffer_);
}
AppendFile::~AppendFile() {
    ::fclose(fp_);
}

void AppendFile::append(const char* logline, const size_t len) {
    size_t written = 0;

    while (written != len) {
        // 剩余字节数
        size_t remain = len - written;
        size_t n = write(logline + written, remain);
        if (n != remain) {
            int err = ferror(fp_);
            if (err) {
                fprintf(
                    stderr, "AppendFile::append() failed %s\n",
                    strerror_tl(err));
                break;
            }
        }
        // 已写入字节数
        written += n;
    }
    // 该文件已写入的总字节数
    writtenBytes_ += written;
}

size_t AppendFile::write(const char* logline, size_t len) {
    // 使用的是不加锁的write,因为是单线程写
    return ::fwrite_unlocked(logline, 1, len, fp_);
}

void AppendFile::flush() {
    ::fflush(fp_);
}

} // namespace tinyrpc