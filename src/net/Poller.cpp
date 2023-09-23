#include "Poller.h"
#include "DateTime.h"
#include "Channel.h"

#include "EventLoop.h"
#include <cassert>
#include <poll.h>
#include <Logging.h>
#include <sys/poll.h>

namespace tinyrpc {
Poller::Poller(EventLoop* loop) :
    ownerLoop_(loop) {
}

Poller::~Poller() {
}

DateTime Poller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    DateTime now;
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        LOG_TRACE << " nothing happended";
    } else {
        LOG_ERROR << "Poller::poll()";
    }
    return now;
}

void Poller::fillActiveChannels(int numEvents,
                                ChannelList* activeChannels) const {
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
        if (pfd->revents > 0) {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel) {
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    if (channel->index() < 0) {
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    } else {
        // update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            // 可以直接忽略这个fd
            pfd.fd = -1;
        }
    }
}
void Poller::removeChannel(Channel* channel) {
    assertInLoopThread();
    //从channels中移除
    LOG_TRACE << "fd = " << channel->fd();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd& pfd = pollfds_[idx];
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1);

    // 从vector中移除
    if (static_cast<size_t>(idx) == pollfds_.size() - 1) { // 在vector的最后
        pollfds_.pop_back();
    } else { // 不在最后
        int channelAtEnd = pollfds_.back().fd;
        // 将要移除的channel放到最后
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd - 1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back(); // 移除
    }
}

} // namespace tinyrpc