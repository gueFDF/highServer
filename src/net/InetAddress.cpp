#include "InetAddress.h"
#include "Logging.h"
#include <cstdint>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
namespace tinyrpc {
// IPv4 地址中的通配地址
static const in_addr_t kInaddrAny = INADDR_ANY;

InetAddress::InetAddress(uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(kInaddrAny);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr)) {
        LOG_SYSERR << "sockets::fromHostPort";
    }
}

std::string InetAddress::toHostPort() const {
    char buf[32];
    char host[INET_ADDRSTRLEN] = "INVALID";
    struct sockaddr_in addr = addr_;
    ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof(host));
    uint16_t port = ::ntohs(addr.sin_port);
    snprintf(buf, 32, "%s:%u", host, port);

    return buf;
}

struct sockaddr_in getLocalAddr(int sockfd) {
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = sizeof(localaddr);
    if (::getsockname(sockfd, (struct sockaddr*)(&localaddr), &addrlen) < 0) {
        LOG_SYSERR << "sockets::getLocalAddr";
    }
    return localaddr;
}
} // namespace tinyrpc