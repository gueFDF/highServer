#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <stdio.h>
#include <unistd.h>

std::string message;

void onConnection(const tinyrpc::TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
    conn->send(message);
  }
  else
  {
    printf("onConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

void onWriteComplete(const tinyrpc::TcpConnectionPtr& conn)
{
  conn->send(message);
}

void onMessage(const tinyrpc::TcpConnectionPtr& conn,
               tinyrpc::Buffer* buf)
{
  printf("onMessage(): received %zd bytes from connection [%s]",
         buf->readableBytes(),
         conn->name().c_str());

  buf->retrieveAll();
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  std::string line;
  for (int i = 33; i < 127; ++i)
  {
    line.push_back(char(i));
  }
  line += line;

  for (size_t i = 0; i < 127-33; ++i)
  {
    message += line.substr(i, 72) + '\n';
  }

  tinyrpc::InetAddress listenAddr(9981);
  tinyrpc::EventLoop loop;

  tinyrpc::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.setWriteCompleteCallback(onWriteComplete);
  server.start();

  loop.loop();
}
