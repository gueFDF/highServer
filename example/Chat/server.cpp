#include "TcpServer.h"
#include "code.h"
#include <cstdio>
#include <functional>
#include "EventLoop.h"
#include <set>
class ChatServer : highServer::noncopyable {
public:
    ChatServer(highServer::EventLoop* loop,
               const highServer::InetAddress& listenAddr) :
        server_(loop, listenAddr),
        codec_(std::bind(&ChatServer::onStringMessage, this,
                         std::placeholders::_1, std::placeholders::_2)) {
        server_.setConnectionCallback(std::bind(&ChatServer::onConnection,
                                                this, std::placeholders::_1));
        server_.setMessageCallback(
            std::bind(&Codec::onMessage, &codec_, std::placeholders::_1,
                      std::placeholders::_2));
    }
    void start() {
        server_.setThreadNum(10);
        server_.start();
    }

private:
    void onConnection(const highServer::TcpConnectionPtr& conn) {
        LOG_INFO << conn->peerAddress().toIpPort() << " -> "
                 << conn->localAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected()) {
            connections_.insert(conn);
        } else {
            connections_.erase(conn);
        }
    }
    void onStringMessage(const highServer::TcpConnectionPtr&,
                         const std::string& message) {
        for (ConnectionList::iterator it = connections_.begin();
             it != connections_.end();
             ++it) {
                printf("message:%s\n",message.c_str());
            codec_.send(*it, message);
        }
    }

    typedef std::set<highServer::TcpConnectionPtr> ConnectionList;
    highServer::TcpServer server_;
    Codec codec_;
    ConnectionList connections_;
};

int main(int argc, char* argv[]) {
    LOG_INFO << "pid = " << ::getpt();
    if (argc > 1) {
        highServer::EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        highServer::InetAddress serverAddr(port);
        ChatServer server(&loop, serverAddr);
        server.start();
        loop.loop();
    } else {
        printf("Usage: %s port\n", argv[0]);
    }
}