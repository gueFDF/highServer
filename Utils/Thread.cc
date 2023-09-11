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
    // 统治父线程启动成功,执行函数
    data->func_();

    delete data;
    return NULL;
}

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
        // TODO: 确保线程启动成功
    }
}

int Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}