#ifndef EVENTLOOP
#define EVENTLOOP
#include "DateTime.h"
#include "TimerQueue.h"
#include "noncopyable.h"
#include <memory>
#include "CurrentThread.h"

namespace tinyrpc {
class Channel;
class Poller;

class EventLoop : tinyrpc::noncopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    DateTime pollReturnTime() const {
        return pollReturnTime_;
    }
    // 定时任务
    TimerId runAt(const DateTime& time, const TimerCallback& cb); // 在time触发
    TimerId runAfter(double delay, const TimerCallback& cb);      // delay时间后触发
    TimerId runEvery(double interval, const TimerCallback& cb);   // 周期性触发

    void updateChannel(Channel* channel);
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
    typedef std::vector<Channel*> ChannelList;
    bool quit_;
    bool looping_;
    const pid_t threadId_;
    DateTime pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    ChannelList activeChannels_;
};

} // namespace tinyrpc

#endif /* EVENTLOOP */
