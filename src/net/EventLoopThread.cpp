#include "EventLoopThread.h"
#include "EventLoop.h"
#include <cassert>
#include <cstdio>
#include <mutex>
namespace highServer {
EventLoopThread::EventLoopThread() :
    loop_(NULL),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this)),
    mutex_(),
    cond_() {
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    loop_->quit();
    thread_.join();
}

EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();

    mutex_.lock();
    while (loop_ == NULL) {
        mutex_.unlock();
        cond_.wait();
        mutex_.lock();
    }
    mutex_.unlock();
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();
    assert(exiting_);
}

} // namespace highServer