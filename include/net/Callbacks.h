#ifndef CALLBACKS
#define CALLBACKS
#include <functional>
#include <memory>
namespace tinyrpc {
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void()> TimerCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&,
                           const char* data,
                           ssize_t len)>
    MessageCallback;

typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;

} // namespace tinyrpc

#endif /* CALLBACKS */
