#ifndef CONNECTOR
#define CONNECTOR
#include "InetAddress.h"
#include "TimerId.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
namespace tinyrpc {

class Channel;
class EventLoop;

class Connector : noncopyable {
public:
    typedef std::function<void(int sockfd)> NewConnectionCallback;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    void start();   // can be called in any thread
    void restart(); // must be called in loop thread
    void stop();    // can be called in any thread

    const InetAddress& serverAddress() const {
        return serverAddr_;
    }

private:
    enum States { kDisconnected,
                  kConnecting,
                  kConnected };
    static const int kMaxRetryDelayMs = 30 * 1000; // 最大重试时间
    static const int kInitRetryDelayMs = 500;      // 重试初始时间

    void setState(States s) {
        state_ = s;
    };
    void startInLoop();
    void connect();              // 进行连接
    void connecting(int sockfd); // connect成功创建channel,加入loop
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    InetAddress serverAddr_;
    bool connect_;
    States state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
    TimerId timerId_;
};
typedef std::shared_ptr<Connector> ConnectorPtr;
} // namespace tinyrpc

#endif /* CONNECTOR */
