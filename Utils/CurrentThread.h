#ifndef CURRENTTHREAD
#define CURRENTTHREAD
#include "Thread.h"
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
/*用来记录当前线程的信息*/

// 可以不用反复从内核曾获取tid
inline int tid() {
    if (t_cachedTid == 0) {
        t_cachedTid = Gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d ", t_cachedTid);
    }

    return t_cachedTid;
}

inline const char* tidString() // for logging
{
    return t_tidString;
}

inline int tidStringLength() // for logging
{
    return t_tidStringLength;
}



#endif /* CURRENTTHREAD */
