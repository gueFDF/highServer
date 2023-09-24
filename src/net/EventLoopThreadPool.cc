#include "EventLoopThreadPool.h"
#include "Channel.h"
#include <cassert>
#include "EventLoop.h"
#include "EventLoopThread.h"
namespace tinyrpc {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop) :
    baseLoop_(baseLoop),
    started_(false),
    numThreads_(0),
    next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
}

void EventLoopThreadPool::start() {
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; i++) {
        EventLoopThread*t=new EventLoopThread();
        threads_.push_back(EventLoopThreadPtr(t));
        loops_.push_back(t->startLoop());
    }
}

//返回一个loopthread 如果没有,则使用主loop
EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        next_ %= loops_.size();
    }

    return loop;
}

} // namespace tinyrpc