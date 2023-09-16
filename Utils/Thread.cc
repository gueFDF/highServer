#include "Thread.h"
#include <cassert>
#include <errno.h>
#include <mutex>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "CurrentThread.h"

namespace tinyrpc {

pid_t Gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

__thread int t_cachedTid = 0;

// 线程启动后,统一调用startThread,然后在执行func
void* startThread(void* obj) {
    // 已经进入子线程
    ThreadData* data = static_cast<ThreadData*>(obj);
    *data->tid_ = gettid();
    // t_threadName=
    data->cond_.notify();
    data->func_();

    delete data;
    return NULL;
}

Thread::Thread(ThreadFunc func, const std::string& n) :
    started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    latch_(mutex_) {
}

// 当主线程结束时,需要detach子线程
Thread::~Thread() {
    if (started_ && !joined_) {
        pthread_detach(pthreadId_);
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(func_, &tid_, latch_);

    if (pthread_create(&pthreadId_, NULL, &startThread, data)) {
        started_ = false;
        delete data;
    } else {
        // 获取子进程通知
        latch_.wait();
    }
}

int Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}
} // namespace tinyrpc