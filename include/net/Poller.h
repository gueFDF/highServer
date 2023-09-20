#ifndef POLLER
#define POLLER
#include <map>
#include <vector>

#include "Channel.h"
#include "DateTime.h"
#include "EventLoop.h"
#include "noncopyable.h"

struct pollfd;
namespace tinyrpc {

class Channel;
class Poller : noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;
    Poller(EventLoop* loop);
    ~Poller();

    DateTime poll(int timeoutMs, ChannelList* activeChannels);
    void updateChannel(Channel* channel);

    void assertInLoopThread() {
        ownerLoop_->assertInLoopThread();
    }

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};

} // namespace tinyrpc
#endif /* POLLER */
