#include "Channel.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "code.h"
#include <iostream>
#include <mutex>
#include <unistd.h>

class ChatClient : highServer::noncopyable {
public:
    ChatClient(highServer::EventLoop* loop,
               const highServer::InetAddress& serverAddr) :
        client_(loop, serverAddr),
        code_(std::bind(&ChatClient::onStringMessage, this, std::placeholders::_1,
                        std::placeholders::_2)) {
        client_.setConnectionCallback(
            std::bind(&ChatClient::onConnection, this, std::placeholders::_1));
        client_.setMessageCallback(
            std::bind(&Codec::onMessage, &code_, std::placeholders::_1,
                      std::placeholders::_2));
        client_.enableRetry();
    }

    void connect() {
        client_.connect();
    }

    void disconnect() {
        client_.disconnect();
    }

    void write(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection_) {
            code_.send(connection_, message);
        }
    }

private:
    void onConnection(const highServer::TcpConnectionPtr& conn) {
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
                 << conn->peerAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");

        std::lock_guard<std::mutex> lock(mutex_);
        if (conn->connected()) {
            connection_ = conn;
        } else {
            connection_.reset();
        }
    }

    void onStringMessage(const highServer::TcpConnectionPtr&,
                         const std::string& message) {
        printf("<<< %s\n", message.c_str());
    }

public:
    highServer::TcpClient client_;
    Codec code_;
    std::mutex mutex_;
    highServer::TcpConnectionPtr connection_;
};

int main(int argc, char* argv[]) {
    LOG_INFO << "pid = " << ::getpid();
    if (argc > 2) {
        highServer::EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        highServer::InetAddress serverAddr(argv[1], port);

        ChatClient client(loopThread.startLoop(), serverAddr);
        client.connect();
        std::string line;
        while (std::getline(std::cin, line)) {
            client.write(line);
        }
        client.disconnect();
    } else {
        printf("Usage: %s host_ip port\n", argv[0]);
    }
}
