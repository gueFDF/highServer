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
class EventLoopThreadPool;

class TcpServer : noncopyable {
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr);
    ~TcpServer();
    void start();
    void setThreadNum(int numThreads);
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
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);
    EventLoop* loop_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool started_;
    int nextConnId_;
    ConnectionMap connections_;
};
} // namespace tinyrpc

#endif /* TCPSERVER */
