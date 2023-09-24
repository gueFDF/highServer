#ifndef TCPCONNECTION
#define TCPCONNECTION

#include "Condition.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include <memory>
#include <string>
#include "Buffer.h"
#include "Callbacks.h"
namespace tinyrpc {
class Channel;
class EventLoop;
class Socket;

// 继承智能
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);

    ~TcpConnection();

    EventLoop* getLoop() const {
        return loop_;
    }
    const std::string& name() const {
        return name_;
    }
    const InetAddress& localAddress() {
        return localAddr_;
    }
    const InetAddress& peerAddress() {
        return peerAddr_;
    }
    bool connected() const {
        return state_ == kConnected;
    }

    // 发送数据
    void send(const std::string& message);
    // 关闭
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }
    void setCloseCallback(const CloseCallback& cb) {
        closeCallback_ = cb;
    }

    void connectEstablished(); // 新连接进行回调
    void connectDestroyed();   // 销毁连接进行回调

private:
    enum StateE { kConnecting,
                  kConnected,
                  kDisconnected
    };
    void setState(StateE s) {
        state_ = s;
    }
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const std::string& message);
    void shutdownInLoop();

    EventLoop* loop_;
    std::string name_;
    StateE state_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    InetAddress localAddr_;
    InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
} // namespace tinyrpc

#endif /* TCPCONNECTION */
