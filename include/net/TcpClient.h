#ifndef TCPCLIENT
#define TCPCLIENT

#include "Callbacks.h"
#include "TcpConnection.h"
#include "noncopyable.h"
#include <memory>
#include <mutex>
namespace highServer {
class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;
class TcpClient : noncopyable {
public:
    TcpClient(EventLoop* loop,
              const InetAddress& serverAddr);
    ~TcpClient();
    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    bool retry() const {
        return retry_;
    }
    void enableRetry() {
        retry_ = true;
    }

    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;
    bool connect_;

    int nextConnId_;
    mutable std::mutex mutex_;
    TcpConnectionPtr connection_;
};

} // namespace highServer

#endif /* TCPCLIENT */
