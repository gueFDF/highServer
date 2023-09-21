#ifndef EVENTLOOPTHREAD
#define EVENTLOOPTHREAD
#include "Condition.h"
#include "Thread.h"
#include "noncopyable.h"
namespace tinyrpc {
class EventLoop;

// 一个eventloop线程类
class EventLoopThread : noncopyable {
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    Condition cond_;
};

} // namespace tinyrpc

#endif /* EVENTLOOPTHREAD */
