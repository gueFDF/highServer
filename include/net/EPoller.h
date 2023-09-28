#ifndef EPOLLER
#define EPOLLER
#include <map>
#include <vector>

#include "Channel.h"
#include "DateTime.h"
#include "EventLoop.h"
#include "noncopyable.h"
#include <sys/epoll.h>

namespace tinyrpc {

class Channel;
class EPoller : noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;
    EPoller(EventLoop* loop);
    ~EPoller();

    DateTime poll(int timeoutMs, ChannelList* activeChannels);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInLoopThread() {
        ownerLoop_->assertInLoopThread();
    }

private:
    static const int kInitEventListSize = 16;
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);
    typedef std::vector<struct epoll_event> EventList;

    EventLoop* ownerLoop_;
    int epollfd_;
    EventList events_;

};

} // namespace tinyrpc
#endif /* EPOLLER */
