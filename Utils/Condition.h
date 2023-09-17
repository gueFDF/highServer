#ifndef CONDITION
#define CONDITION
#include "noncopyable.h"
#include <cassert>
#include <cstdlib>
#include <mutex>
#include <pthread.h>
#include <condition_variable>

namespace tinyrpc {
class Condition : noncopyable {
public:
    explicit Condition() :
        mutex_(), cond_() {
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

    bool waitForSeconds(int seconds) {
        std::unique_lock<std::mutex> lck(mutex_);
        std::chrono::seconds timeout(seconds);

        auto ret = cond_.wait_for(lck, timeout);
        if (ret == std::cv_status::timeout) {
            return false;
        }
        return true;
    }

private:
    std::mutex mutex_;
    std::condition_variable cond_;
};

} // namespace tinyrpc
#endif /* CONDITION */
