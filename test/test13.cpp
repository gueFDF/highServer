#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpClient.h"

#include "Logging.h"


#include <utility>

#include <stdio.h>
#include <unistd.h>

std::string message = "Hello\n";

void onConnection(const highServer::TcpConnectionPtr& conn) {
    if (conn->connected()) {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());
        conn->send(message);
    } else {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }
}

void onMessage(const highServer::TcpConnectionPtr& conn,
               highServer::Buffer* buf) {
    printf("onMessage(): received %zd bytes from connection [%s] \n",
           buf->readableBytes(),
           conn->name().c_str());

    printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main() {
    highServer::EventLoop loop;
    highServer::InetAddress serverAddr("127.0.0.1", 9981);
    highServer::TcpClient client(&loop, serverAddr);

    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    client.enableRetry();
    client.connect();
    loop.loop();
}
