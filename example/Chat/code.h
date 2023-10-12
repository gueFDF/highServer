#ifndef CODE
#define CODE

#include "Callbacks.h"
#include "Logging.h"
#include "TcpConnection.h"
#include "noncopyable.h"
#include <cstdint>
#include <cstdio>
#include <endian.h>
#include <functional>
#include "Buffer.h"
class Codec : highServer::noncopyable {
public:
    typedef std::function<void(const highServer::TcpConnectionPtr&,
                               const std::string& message)>
        StringMessageCallback;
    explicit Codec(const StringMessageCallback& cb) :
        messageCallback_(cb) {
    }

    void onMessage(const highServer::TcpConnectionPtr& conn,
                   highServer::Buffer* buf) {
        while (buf->readableBytes() >= kHeaderLen) {
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t*>(data);
            const int32_t len = be32toh(be32);
            if (len > 65536 || len < 0) {
                LOG_ERROR << "Invalid length " << len;
                conn->shutdown();
                break;
            } else if (buf->readableBytes() >= len + kHeaderLen) {
                buf->retrieve(kHeaderLen);
                std::string message(buf->peek(), len);
                messageCallback_(conn, message);
                buf->retrieve(len);
            } else { // 不是一条完整的消息,等待下一次被触发
                break;
            }
        }
    }
    void send(highServer::TcpConnectionPtr conn,
              const std::string& message) {
        highServer::Buffer buf;
        buf.append(message.data(), message.size());
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = htobe32(len);
        buf.prepend(&be32, sizeof(be32));
        conn->send(std::string(buf.peek(), buf.readableBytes()));
    }

private:
    StringMessageCallback messageCallback_;
    const static size_t kHeaderLen = sizeof(int32_t); // 4字节
};

#endif /* CODE */
