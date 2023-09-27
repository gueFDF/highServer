#include "Socket.h"
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>
#include "InetAddress.h"
#include <netinet/tcp.h>
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

// 套接字选项(0关闭,1开启)

// 端口复用
void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
}

// 禁用 Nagle 算法
void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
#else
    if (on) {
        LOG_ERROR << "SO_REUSEPORT is not supported."; // 不支持
    }
#endif
}

// 半关闭(不能写数据,但是可以收数据)
void Socket::shutdownWrite() {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_SYSERR << "sockets::shutdownWrite";
    }
}

} // namespace tinyrpc