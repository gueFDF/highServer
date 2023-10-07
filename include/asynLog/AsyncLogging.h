#ifndef ASYNCLOGGING
#define ASYNCLOGGING
#include "FixedBuffer.h"
#include "Logging.h"
#include "noncopyable.h"
#include <cassert>
#include <mutex>
#include <vector>
#include "Thread.h"
#include "LogStream.h"
#include "Condition.h"
namespace highServer {
class AsyncLogging;
extern __thread AsyncLogging* g_asyncLog;
class AsyncLogging : noncopyable {
public:
    AsyncLogging(const std::string& basename = "test",
                 off_t rollSize = 500 * 1000 * 1000,
                 int flushInterval = 3);

    ~AsyncLogging() {
        if (running_) {
            stop();
        }
    }

    void append(const char* logline, int len);
    void start();

    void stop() {
        running_ = false;
        cond_.notify();
        thread_.join();
    }
    static void StartAsyncLog();

private:
    void threadFunc();
    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector; // 存放多个缓冲区
    typedef BufferVector::value_type BufferPtr;
    const int flushInterval_;
    std::atomic<bool> running_;
    const std::string basename_;
    const off_t rollSize_;
    Thread thread_;
    std::mutex mutex_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_; // 用来存放被填满,待写入的缓冲区
    Condition cond_;       // 用来线程间同步
};

} // namespace highServer

#endif /* ASYNCLOGGING */
