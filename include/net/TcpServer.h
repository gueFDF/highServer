#ifndef TCPSERVER
#define TCPSERVER
#include "Callbacks.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include <memory>
#include <string>
#include <map>
#include <TcpConnection.h>
namespace tinyrpc {
class Acceptor;
class EventLoop;

class TcpServer : noncopyable {
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr);
    ~TcpServer();
    void start();
    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop* loop_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    bool started_;
    int nextConnId_;
    ConnectionMap connections_;
};
} // namespace tinyrpc

#endif /* TCPSERVER */
