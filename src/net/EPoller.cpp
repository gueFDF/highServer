#include "EPoller.h"
#include "DateTime.h"
#include "Channel.h"

#include "EventLoop.h"
#include <cassert>
#include <cstdio>
#include <poll.h>
#include <Logging.h>
#include "Socket.h"
#include <sys/epoll.h>
#include <sys/poll.h>

namespace highServer {

const int kNew = -1;  // 未添加
const int kAdded = 1; // 已添加

EPoller::EPoller(EventLoop* loop) :
    ownerLoop_(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_SYSFATAL << "EPoller::EPoller";
    }
}

EPoller::~EPoller() {
    Close(epollfd_);
}

DateTime EPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                                 static_cast<int>(events_.size()), -1);
    DateTime now;

    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happended";
        // 扩容,增长因子为2
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == static_cast<int>(events_.size())) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        LOG_TRACE << " nothing happended";
    } else {
        LOG_SYSERR << "EPoller::poll()";
    }
    return now;
}

void EPoller::fillActiveChannels(int numEvents,
                                 ChannelList* activeChannels) const {
    assert(numEvents <= events_.size());
    for (int i = 0; i < numEvents; i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPoller::updateChannel(Channel* channel) {
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();

    const int index = channel->index();
    const int fd = channel->fd();
    if (index == kNew) {
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // has existing
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kNew);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
void EPoller::removeChannel(Channel* channel) {
    assertInLoopThread();
    // 从channels中移除
    LOG_TRACE << "remove Channel fd = " << channel->fd();
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(idx == kAdded);
    // 还未在监听红黑树上删除
    if (idx == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }

    // 修改为未添加状态
    channel->set_index(kNew);
}

void EPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op=" << operation << " fd=" << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op=" << operation << " fd=" << fd;
        }
    }
}

} // namespace highServer