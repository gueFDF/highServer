#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <stdio.h>
#include <unistd.h>

void onConnection(const tinyrpc::TcpConnectionPtr& conn) {
    if (conn->connected()) {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());
    } else {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }
}

void onMessage(const tinyrpc::TcpConnectionPtr& conn,
               const char* data,
               ssize_t len) {
    printf("onMessage(): received %zd bytes from connection [%s]\n",
           len, conn->name().c_str());
}

int main() {
    printf("main(): pid = %d\n", getpid());

    tinyrpc::InetAddress listenAddr(9981);
    tinyrpc::EventLoop loop;

    tinyrpc::TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}