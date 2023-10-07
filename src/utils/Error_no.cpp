#include <cstring>
#include "Error_no.h"
namespace highServer {
// 一个线程安全的错误信息获取方法
__thread char t_errnobuf[512]; // 记录错误信息

const char* strerror_tl(int savedErrno) {
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

} // namespace highServer