#ifndef EVENTLOOPTHREADPOOL
#define EVENTLOOPTHREADPOOL

#include "Condition.h"
#include "EventLoopThread.h"
#include "Thread.h"
#include "noncopyable.h"
#include <memory>
#include <mutex>
#include <vector>
namespace tinyrpc {
class EventLoop;

class EventLoopThreadPool : noncopyable {
public:
    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) {
        numThreads_ = numThreads;
    }
    void start();
    EventLoop* getNextLoop();

private:
    typedef std::unique_ptr<EventLoopThread> EventLoopThreadPtr;
    typedef std::vector<EventLoopThreadPtr> ptr_vector;
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_; // always in loop thread
    ptr_vector threads_;
    std::vector<EventLoop*> loops_;
};

} // namespace tinyrpc

#endif /* EVENTLOOPTHREADPOOL */
