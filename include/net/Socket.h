#ifndef SOCKET
#define SOCKET
#include "noncopyable.h"
namespace tinyrpc {
class InetAddress;
int createNBListenSocket();
void Close(int fd);
int getSocketError(int sockfd);
class Socket : noncopyable {
public:
    explicit Socket(int sockfd) :
        sockfd_(sockfd) {
    }
    ~Socket();

    int fd() const {
        return sockfd_;
    }

    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);
    void setReuseAddr(bool on);  // 设置端口复用(断开快速启动,放置time_wait)
    void setTcpNoDelay(bool on); // 设置Nodelay(该选项用来关闭Nagle算法)
    void setKeepAlive(bool on);  // keepalive (定期检查网络状态)
    void setReusePort(bool on);  // 端口重复绑定
    void shutdownWrite();

private:
    const int sockfd_;
};

} // namespace tinyrpc

#endif /* SOCKET */
