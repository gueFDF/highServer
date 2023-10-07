#include <cassert>
#include <cstddef>
#include <cstdio>
#include <sys/timerfd.h>
#include <unistd.h>
#include <vector>
#include "DateTime.h"
#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "Logging.h"
#include "TimerId.h"
namespace highServer {
int createTimerfd() {
    // 创建一个定时文件描述符
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

void readTimerfd(int timerfd, DateTime now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toIsoString();
    if (n != sizeof(howmany)) {
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}

struct timespec howMuchTimeFromNow(DateTime when) {
    int64_t microseconds = when.gettime() - DateTime().gettime();
    if (microseconds < 100) {
        microseconds = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / (1000 * 1000));
    ts.tv_nsec = static_cast<time_t>(microseconds % (1000 * 1000) * 1000);

    return ts;
}

void resetTimerfd(int timerfd, DateTime expiration) {
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);

    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        LOG_SYSERR << "timerfd_settime()";
    }
}

TimerQueue::TimerQueue(EventLoop* loop) :
    loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_(),
    callingExpiredTimers_(false) {
    timerfdChannel_.setReadCallback(
        std::bind(&TimerQueue::handleRead, this));
    // 设置读事件
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);
    for (TimerList::iterator it = timers_.begin();
         it != timers_.end(); ++it) {
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
                             DateTime when,
                             double interval) {
    Timer* timer = new Timer(cb, when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(
        std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    // 如果最早被触发的定时器被改变,重置timerfd_的超时时间(防止导致,有任务到期,timerfd_还未到期)
    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == timers_.size());
    ActiveTimer timer(timerId.value_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    // 如果在,就删除
    if (it != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        delete it->first;
        activeTimers_.erase(it);
    } else if (callingExpiredTimers_) { // 用来应对在定时器回调当中,cancel
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    DateTime now;
    readTimerfd(timerfd_, now); // 将数据读出来,放置忙轮询
    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear(); // 清除
    for (std::vector<Entry>::iterator it = expired.begin(); it != expired.end(); ++it) {
        it->second->run(); // 执行该定时任务(对应的回调)
    }

    callingExpiredTimers_ = false;

    reset(expired, now);
}

// 获取所有到期任务
std::vector<TimerQueue::Entry> TimerQueue::getExpired(DateTime now) {
    std::vector<Entry> expired;
    // 第二个元素没必要,set只会比较第一个元素
    Entry sentry = std::make_pair(now, nullptr);
    // 使用二分查找第一个DateTime大于等于now的值
    TimerList::iterator it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);
    // back_inserter(expired) 是一个插入迭代器,copy时不包括it
    std::copy(timers_.begin(), it, back_inserter(expired));
    timers_.erase(timers_.begin(), it); // 删除

    // 从activeTimerSet_中删除
    for (std::vector<Entry>::const_iterator it_ = expired.begin();
         it_ != expired.end(); ++it_) {
        ActiveTimer timer(it_->second, it_->second->sequence());
        size_t n = activeTimers_.erase(timer);
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, DateTime now) {
    DateTime nextExpire;
    nextExpire.invalid(); // 置为无效
    for (std::vector<Entry>::const_iterator it = expired.begin(); it != expired.end(); ++it) {
        ActiveTimer timer(it->second, it->second->sequence());
        // 设置为重复触发且没有被cancel
        if (it->second->repeat()
            && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            it->second->restart(now);
            insert(it->second);
        } else {
            delete it->second;
        }
    }
    if (!timers_.empty()) {
        nextExpire = timers_.begin()->second->expiration();
    }

    // 根据最早到期的定时任务重新设置timefd_
    if (nextExpire.valid()) {
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer* timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    DateTime when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    // 判断是否将会成为最小元素
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    timers_.insert(std::make_pair(when, timer));

    activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

} // namespace highServer