#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
namespace tinyrpc {
Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr) :
    loop_(loop),
    acceptSocket_(createNBListenSocket()),
    acceptChannel_(loop, acceptSocket_.fd()),
    listenning_(false) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(
        std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading(); // 设置事件并注册到poll
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        //如果注册回调,进行回调
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            Close(connfd);
        }
    }
}
} // namespace tinyrpc