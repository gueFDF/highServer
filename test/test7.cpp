#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "unistd.h"
#include <stdio.h>

void newConnection(int sockfd, const highServer::InetAddress& peerAddr) {
    printf("newConnection(): accepted a new connection from %s\n",
           peerAddr.toHostPort().c_str());
    ::write(sockfd, "How are you?\n", 13);
    highServer::Close(sockfd);
}

int main() {
    printf("main(): pid = %d\n", getpid());

    highServer::InetAddress listenAddr(9981);
    highServer::EventLoop loop;

    highServer::Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();

    loop.loop();
}
