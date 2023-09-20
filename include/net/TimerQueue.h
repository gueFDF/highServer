#ifndef TIMERQUEUE
#define TIMERQUEUE
#include "DateTime.h"
#include "noncopyable.h"
#include <set>
#include <utility>
#include <vector>
#include "Channel.h"
#include "Callbacks.h"
namespace tinyrpc {
class EventLoop;
class Timer;
class TimerId;

// 定时器
class TimerQueue : noncopyable {
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();
    TimerId addTimer(const TimerCallback& cb,
                     DateTime when,
                     double interval); // 添加定时任务

private:
    typedef std::pair<DateTime, Timer*> Entry;
    typedef std::set<Entry> TimerList;
    void handleRead(); //处理
    std::vector<Entry> getExpired(DateTime now);                 // 获取到期的Entry
    void reset(const std::vector<Entry>& expired, DateTime now); // 重新设置
    bool insert(Timer* timer);
    EventLoop* loop_;
    const int timerfd_; // 整个定时器由timerfd_驱动
    Channel timerfdChannel_;
    TimerList timers_;
};

} // namespace tinyrpc

#endif /* TIMERQUEUE */
