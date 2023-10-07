#include "EventLoop.h"
#include "CurrentThread.h"
#include "AsyncLogging.h"
#include "DateTime.h"
#include "Logging.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <poll.h>
#include "EPoller.h"
#include <sys/eventfd.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "TimerId.h"
namespace highServer {

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

// 用来忽略sigpipe信号
class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
IgnoreSigPipe initObj;

EventLoop::EventLoop() :
    quit_(false),
    looping_(false),
    callingPendingFunctors_(false),
    threadId_(tid()),
    epoller_(new EPoller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)) {
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    } else {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    assert(!looping_);
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        epoller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handleEvent();
        }

        // 执行其他线程append到Functors中的函数
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

// 在loop线程中执行某些函数
void EventLoop::runInLoop(const Functor& cb) {
    // 如果在loopthread中就直接执行
    if (isInLoopThread()) {
        cb();
    } else {
        // 不在loopthread就加在执行队列并唤醒
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
    // 只有在当前线程执行事件回调时才不需要wakeup(因为执行完,会去执行callingPending函数)
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

TimerId EventLoop::runAt(const DateTime& time, const TimerCallback& cb) {
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb) {
    DateTime a = DateTime();
    DateTime time(addTime(DateTime(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb) {
    DateTime time(addTime(DateTime(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::cancel(TimerId timerId) {
    return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << tid();
}

// 唤醒loop(可能阻塞在pool/epoll,向eventfd写数据)
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

// 将eventfd的数据(其实只是一个计数)读出来,防止在水平模式下忙轮询
void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

// 执行Functors中的函数
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functions;
    // 先交换,再依次执行func,目的是快速出临界区
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functions.swap(pendingFunctors_);
    }
    for (size_t i = 0; i < functions.size(); ++i) {
        functions[i]();
    }
    callingPendingFunctors_ = false;
}

} // namespace highServer
