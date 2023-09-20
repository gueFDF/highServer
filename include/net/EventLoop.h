#ifndef EVENTLOOP
#define EVENTLOOP
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
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;
};

} // namespace tinyrpc

#endif /* EVENTLOOP */
