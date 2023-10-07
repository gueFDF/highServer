#ifndef TIMER
#define TIMER
#include "DateTime.h"
#include "noncopyable.h"
#include "Callbacks.h"
#include <atomic>
namespace highServer {

class Timer : noncopyable {
public:
    Timer(const TimerCallback& cb, DateTime when, double interval) :
        callback_(cb),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0),
        sequence_(s_numCreated_.fetch_add(1)) {
    }

    void run() const {
        callback_();
    }

    DateTime expiration() const {
        return expiration_;
    }
    bool repeat() const {
        return repeat_;
    }
    int64_t sequence() const {
        return sequence_;
    }

    void restart(DateTime now);

private:
    const TimerCallback callback_;
    DateTime expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic_int64_t s_numCreated_;
};

} // namespace highServer

#endif /* TIMER */
