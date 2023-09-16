#ifndef THREAD
#define THREAD

#include "noncopyable.h"
#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include <functional>
#include <string>
#include <atomic>
#include "Condition.h"
namespace tinyrpc {

pid_t Gettid();
class Thread : noncopyable {
public:
    typedef std::function<void()> ThreadFunc;
    explicit Thread(ThreadFunc, const std::string& name = std::string());
    ~Thread();

    void start();
    int join(); // return pthread_join()
    bool started() const {
        return started_;
    }

    pid_t tid() const {
        return tid_;
    }

private:
    void setDefaultName();

    bool started_;
    bool joined_;
    pthread_t pthreadId_; // 线程标识符
    pid_t tid_;           // 线程ID
    ThreadFunc func_;
    std::mutex mutex_;
    Condition latch_; // 用来线程间同步
};

// 用来向自线程传递一些信息
struct ThreadData {
public:
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadData(ThreadFunc func, pid_t* tid, Condition& cond) :
        func_(func), tid_(tid), cond_(cond) {
    }

    ThreadFunc func_;
    pid_t* tid_;
    Condition& cond_;
};
} // namespace tinyrpc
#endif /* THREAD */
