#include "Connector.h"
#include "EventLoop.h"

#include <cassert>
#include <stdio.h>

tinyrpc::EventLoop* g_loop;

void connectCallback(int sockfd) {
    printf("connected.\n");
    g_loop->quit();
}

int main(int argc, char* argv[]) {
    tinyrpc::EventLoop loop;
    g_loop = &loop;
    tinyrpc::InetAddress addr("127.0.0.1", 9981);
    tinyrpc::ConnectorPtr connector(new tinyrpc::Connector(&loop, addr));
    connector->setNewConnectionCallback(connectCallback);
    connector->start();

    loop.loop();
}
