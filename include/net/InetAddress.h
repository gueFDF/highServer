#ifndef INETADDRESS
#define INETADDRESS
#include "noncopyable.h"
#include <cstdint>
#include <string>
#include <netinet/in.h>
namespace highServer {
class InetAddress {
public:
    explicit InetAddress(uint16_t port);
    InetAddress(const std::string& ip, uint16_t port);
    InetAddress(const struct sockaddr_in& addr) :
        addr_(addr) {
    }
    std::string toHostPort() const;

    const struct sockaddr_in& getSockAddrInet() const {
        return addr_;
    }
    void setSockAddrInet(const struct sockaddr_in& addr) {
        addr_ = addr;
    }

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

private:
    struct sockaddr_in addr_;
};

struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);
} // namespace highServer

#endif /* INETADDRESS */
