#ifndef SOCKET
#define SOCKET
#include "noncopyable.h"
namespace tinyrpc {
class InetAddress;
int createNBListenSocket();
void Close(int fd);
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

private:
    const int sockfd_;
};

} // namespace tinyrpc

#endif /* SOCKET */