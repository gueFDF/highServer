#ifndef EVENTLOOP
#define EVENTLOOP
#include "DateTime.h"
#include "TimerQueue.h"
#include "noncopyable.h"
#include <memory>
#include <mutex>
#include "CurrentThread.h"

namespace tinyrpc {
class Channel;
class Poller;

class EventLoop : tinyrpc::noncopyable {
public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    DateTime pollReturnTime() const {
        return pollReturnTime_;
    }

    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);
    // 定时任务
    TimerId runAt(const DateTime& time, const TimerCallback& cb); // 在time触发
    TimerId runAfter(double delay, const TimerCallback& cb);      // delay时间后触发
    TimerId runEvery(double interval, const TimerCallback& cb);   // 周期性触发

    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const {
        return threadId_ == tid();
    }

private:
    void abortNotInLoopThread();
    void handleRead(); // waked up
    void doPendingFunctors();

    typedef std::vector<Channel*> ChannelList;
    bool quit_;
    bool looping_;
    bool callingPendingFunctors_;
    const pid_t threadId_;
    DateTime pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_; // 用来唤醒poll/epoll的fd
    std::unique_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;
    std::mutex mutex_; // 保证pendingFunctors_的线程安全
    std::vector<Functor> pendingFunctors_;
};

} // namespace tinyrpc

#endif /* EVENTLOOP */
