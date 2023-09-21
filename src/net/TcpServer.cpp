#include "TcpServer.h"
#include "InetAddress.h"
#include "Logging.h"
#include "noncopyable.h"
#include "Acceptor.h"
#include <functional>
#include <memory>
#include "EventLoop.h"
namespace tinyrpc {

class Acceptor;
class EventLoop;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr) :
    loop_(loop),
    name_(listenAddr.toHostPort()),
    acceptor_(new Acceptor(loop, listenAddr)),
    started_(false),
    nextConnId_(1) {
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
    }

    if (!acceptor_->listenning()) {
        loop_->runInLoop(
            std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_
             << "] - new connection [" << connName
             << "] from " << peerAddr.toHostPort();
    InetAddress localAddr(getLocalAddr(sockfd));

    //创建connection对象
    TcpConnectionPtr conn(
        new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    // 注册
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->connectEstablished(); // 注册到epoll并且执行connectionCallback_
}

} // namespace tinyrpc