#ifndef CHANNEL
#define CHANNEL
#include "noncopyable.h"
#include <functional>
namespace highServer {
class EventLoop;

class Channel : noncopyable {
public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void()> ReadEventCallback;
    Channel(EventLoop* loop, int fd);
    ~Channel();
    void handleEvent();
    void setReadCallback(const ReadEventCallback& cb) {
        readCallback_ = cb;
    }
    void setWriteCallback(const EventCallback& cb) {
        writeCallback_ = cb;
    }
    void setErrorCallback(const EventCallback& cb) {
        errorCallback_ = cb;
    }
    void setCloseCallback(const EventCallback& cb) {
        closeCallback_ = cb;
    }

    int fd() const {
        return fd_;
    }
    int events() const {
        return events_;
    }
    void set_revents(int revt) {
        revents_ = revt;
    }
    bool isNoneEvent() const {
        return events_ == kNoneEvent;
    }
    void enableReading() {
        events_ |= kReadEvent;
        update();
    }
    void enableWriting() {
        events_ |= kWriteEvent;
        update();
    }

    void disableWriting() {
        events_ &= ~kWriteEvent;
        update();
    }
    // 关闭所有事件
    void disableAll() {
        events_ = kNoneEvent;
        update();
    }
    bool isWriting() const {
        return events_ & kWriteEvent;
    }
    int index() {
        return index_;
    }
    void set_index(int idx) {
        index_ = idx;
    }

    EventLoop* ownerLoop() {
        return loop_;
    }

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_; // used by EPoller.

    bool eventHandling_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
};

} // namespace highServer

#endif /* CHANNEL */
