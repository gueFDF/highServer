#include "Thread.h"
#include <cassert>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}


// 线程启动后,统一调用startThread,然后在执行func
void* startThread(void* obj) {
    ThreadData* data = static_cast<ThreadData*>(obj);
    *data->tid_ = gettid();

    data->latch_.set_value();
    data->func_();

    delete data;
    return NULL;
}


std::atomic_uint32_t Thread::numCreated_ = 0;

void Thread::setDefaultName() {
    // fetch_add返回的是增加前的值
    int num = numCreated_.fetch_add(1) + 1;
    if (name_.empty()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}

Thread::Thread(ThreadFunc func, const std::string& n) :
    started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    name_(n) {
    setDefaultName();
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
        auto rst = latch_.get_future();
        rst.get();
    }
}

int Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}