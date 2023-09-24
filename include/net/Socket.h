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
    void setReuseAddr(bool on);

    void shutdownWrite();

private:
    const int sockfd_;
};

} // namespace tinyrpc

#endif /* SOCKET */
