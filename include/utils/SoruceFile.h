#ifndef SORUCEFILE
#define SORUCEFILE
#include <cstring>
// 用来计算文件路径的基名
namespace tinyrpc {

class SourceFile {
public:
    template <int N>
    SourceFile(const char (&arr)[N]) :
        data_(arr),
        size_(N - 1) {
        const char* slash = strrchr(data_, '/'); // builtin function
        if (slash) {
            data_ = slash + 1;
            size_ -= static_cast<int>(data_ - arr);
        }
    }

    explicit SourceFile(const char* filename) :
        data_(filename) {
        const char* slash = strrchr(filename, '/');
        if (slash) {
            data_ = slash + 1;
        }
        size_ = static_cast<int>(strlen(data_));
    }

    const char* data_;
    int size_;
};
} // namespace tinyrpc

#endif /* SORUCEFILE */
