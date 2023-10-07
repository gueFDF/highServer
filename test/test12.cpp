#include "Connector.h"
#include "EventLoop.h"

#include <cassert>
#include <stdio.h>

highServer::EventLoop* g_loop;

void connectCallback(int sockfd) {
    printf("connected.\n");
    g_loop->quit();
}

int main(int argc, char* argv[]) {
    highServer::EventLoop loop;
    g_loop = &loop;
    highServer::InetAddress addr("127.0.0.1", 9981);
    highServer::ConnectorPtr connector(new highServer::Connector(&loop, addr));
    connector->setNewConnectionCallback(connectCallback);
    connector->start();

    loop.loop();
}
