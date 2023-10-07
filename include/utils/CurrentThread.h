#ifndef CURRENTTHREAD
#define CURRENTTHREAD
#include "Thread.h"

namespace highServer {
extern __thread int t_cachedTid;

/*用来记录当前线程的信息*/

// 可以不用反复从内核曾获取tid
inline int tid() {
    if (t_cachedTid == 0) {
        t_cachedTid = Gettid();
    }

    return t_cachedTid;
}
} // namespace highServer

#endif /* CURRENTTHREAD */
