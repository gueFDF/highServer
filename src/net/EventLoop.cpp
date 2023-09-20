#include "EventLoop.h"
#include "CurrentThread.h"
#include "AsyncLogging.h"
#include "DateTime.h"
#include "Logging.h"
#include <cassert>
#include <cstdio>
#include <poll.h>
#include "Poller.h"
#include "TimerId.h"
namespace tinyrpc {

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

EventLoop::EventLoop() :
    quit_(false),
    looping_(false),
    threadId_(tid()),
    poller_(new Poller(this)),
    timerQueue_(new TimerQueue(this)) {
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    } else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handleEvent();
        }
    }

    ::poll(nullptr, 0, 5 * 1000);
    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
}

TimerId EventLoop::runAt(const DateTime& time, const TimerCallback& cb) {
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb) {
    DateTime a = DateTime();
    printf("timea %s\n", a.toIsoString().c_str());
    DateTime time(addTime(DateTime(), delay));
    printf("timeb %s\n", time.toIsoString().c_str());
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb) {
    DateTime time(addTime(DateTime(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << tid();
}

} // namespace tinyrpc
