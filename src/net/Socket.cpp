#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>
#include "InetAddress.h"
#include "Logging.h"
namespace tinyrpc {

int createNBListenSocket() {
    // 监听套接字使用非阻塞的原因是,防止accept阻塞
    // 场景 ,客户端发起connect后,在server在epoll_wait触发后,accept前
    // 客户端连接出现问题,发送rst报文
    // 此时server accept就会阻塞
    int sockfd = ::socket(AF_INET,
                          SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_SYSFATAL << "sockets::createNBListenSocket";
    }

    return sockfd;
}

void Close(int fd) {
    if (::close(fd) < 0) {
        LOG_SYSERR << "sockets::close";
    }
}

// 获取套接字的相关错误状态
int getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = sizeof optval;

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

Socket::~Socket() {
    Close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr) {
    int ret = ::bind(sockfd_, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::bind";
    }
}

void Socket::listen() {
    int ret = ::listen(sockfd_, 4096);
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::listen";
    }
}

int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    socklen_t addrlen = sizeof(client_addr);

    int connfd = ::accept(sockfd_, (struct sockaddr*)&client_addr, &addrlen);
    if (connfd < 0) {
        int savedErrno = errno;
        LOG_SYSERR << "Socket::accept";
    }

    peeraddr->setSockAddrInet(client_addr);
    return connfd;
}

// 0关闭,1开启
void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, sizeof(optval));
}

} // namespace tinyrpc