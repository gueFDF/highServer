#ifndef CONDITION
#define CONDITION
#include "noncopyable.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <pthread.h>
#include <condition_variable>

namespace tinyrpc {
class Condition : noncopyable {
public:
    explicit Condition(std::mutex& mutex) :
        mutex_(mutex) {
    }
    ~Condition() = default;

    void notify() {
        cond_.notify_one();
    }

    void notifyall() {
        cond_.notify_all();
    }

    void wait() {
        std::unique_lock<std::mutex> lck(mutex_);
        cond_.wait(lck);
    }

private:
    std::mutex& mutex_;
    std::condition_variable cond_;
};

} // namespace tinyrpc
#endif /* CONDITION */
