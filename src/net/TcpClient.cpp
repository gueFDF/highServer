#include "TcpClient.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "Connector.h"
#include "InetAddress.h"
#include "Logging.h"
#include <functional>
#include <mutex>

namespace tinyrpc {
void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn) {
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector) {
}

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& serverAddr) :
    loop_(loop),
    connector_(new Connector(loop, serverAddr)),
    retry_(false),
    connect_(false),
    nextConnId_(1) {
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
    LOG_INFO << "TcpClient::TcpClient[" << this
             << "] - connector " << connector_.get();
};

TcpClient::~TcpClient() {
    LOG_INFO << "TcpClient::~TcpClient[" << this
             << "] - connector " << connector_.get();
    TcpConnectionPtr conn;
    mutex_.lock();
    conn = connection_;
    mutex_.unlock();
    if (conn) {
        CloseCallback cb = std::bind(&tinyrpc::removeConnection, loop_, std::placeholders::_1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
    } else {
        connector_->stop();
        loop_->runAfter(1, std::bind(&tinyrpc::removeConnector, connector_));
    }
}

void TcpClient::connect() {
    LOG_INFO << "TcpClient::connect[" << this << "] - connecting to "
             << connector_->serverAddress().toHostPort();

    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toHostPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = buf;
    InetAddress localAddr(getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        std::lock_guard<std::mutex> lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_) {
        LOG_INFO << "TcpClient::connect[" << this << "] - Reconnecting to "
                 << connector_->serverAddress().toHostPort();
        connector_->restart();
    }
}

} // namespace tinyrpc