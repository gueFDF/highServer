#include "Connector.h"
#include "Channel.h"
#include "Error_no.h"
#include "InetAddress.h"
#include "Logging.h"
#include "EventLoop.h"
#include "Socket.h"
#include <cassert>
#include <cstdio>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>

namespace highServer {
const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr) :
    loop_(loop),
    serverAddr_(serverAddr),
    connect_(false),
    state_(kDisconnected),
    retryDelayMs_(kInitRetryDelayMs) {
    LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector() {
    loop_->cancel(timerId_);
    assert(!channel_);
}

void Connector::start() {
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if (connect_) {
        connect();
    } else {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::connect() {
    int sockfd = createNBListenSocket();
    struct sockaddr_in addr = serverAddr_.getSockAddrInet();
    int ret = ::connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        // 以上错误表示正在连接,加入loop进行监听,直至连接成功
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        // 以上连接表示失败,需要关闭socketfd,重新创建并连接
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        // 无可挽回的失败
        LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
        Close(sockfd);
        break;

    default:
        LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
        Close(sockfd);
        break;
    }
}

void Connector::restart() {
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::stop() {
    connect_ = false;
    loop_->cancel(timerId_);
}

void Connector::connecting(int sockfd) {
    // 创建channel,加入监听,触发后检查是否连接成功
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(
        std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallback(
        std::bind(&Connector::handleError, this));

    channel_->enableWriting();
}

// 从loop中移除channel,并且将channel重置为nullptr
int Connector::removeAndResetChannel() {
    channel_->disableAll();
    int sockfd = channel_->fd();
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

// 置空channel
void Connector::resetChannel() {
    channel_.reset();
}

void Connector::handleWrite() {
    if (state_ == kConnecting) {
        LOG_TRACE << "Connector::handleWrite " << state_;

        assert(state_ == kConnecting);
        int sockfd = removeAndResetChannel();
        int err = getSocketError(sockfd);
        // 取消监听,销毁channel
        if (err) {
            // 如果错误,不在等待,进行重试
            LOG_WARN << "Connector::handleWrite - SO_ERROR = "
                     << err << " " << strerror_tl(err);
            retry(sockfd);
        } else if (isSelfConnect(sockfd)) { // 处理自连接
            LOG_WARN << "Connector::handleWrite - Self connect";
            retry(sockfd);
        } else {
            setState(kConnected);
            if (connect_) {
                newConnectionCallback_(sockfd);
            } else { // 已经停止client
                Close(sockfd);
            }
        }
    }
}

void Connector::handleError() {
    LOG_ERROR << "Connector::handleError";
    assert(state_ == kConnecting);
    int sockfd = removeAndResetChannel();
    int err = getSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sockfd);
}

// 重联
void Connector::retry(int sockfd) {
    Close(sockfd); // 先关闭旧的fd,fd需要重新创建
    setState(kDisconnected);
    if (connect_) {
        LOG_INFO << "Connector::retry - Retry connecting to "
                 << serverAddr_.toHostPort() << " in "
                 << retryDelayMs_ << " milliseconds. ";
        // 加入定时队列,到时进行重联
        timerId_ = loop_->runAfter(retryDelayMs_ / 1000.0, std::bind(&Connector::startInLoop, this));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, Connector::kMaxRetryDelayMs);
    } else { // 代表已经关闭
        LOG_DEBUG << "do not connect";
    }
}

} // namespace highServer