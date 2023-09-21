#ifndef INETADDRESS
#define INETADDRESS
#include "noncopyable.h"
#include <cstdint>
#include <string>
#include <netinet/in.h>
namespace tinyrpc {
class InetAddress : noncopyable {
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

private:
    struct sockaddr_in addr_;
};

} // namespace tinyrpc

#endif /* INETADDRESS */
