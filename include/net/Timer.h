#ifndef TIMER
#define TIMER
#include "DateTime.h"
#include "noncopyable.h"
#include "Callbacks.h"
namespace tinyrpc {

class Timer : noncopyable {
public:
    Timer(const TimerCallback& cb, DateTime when, double interval) :
        callback_(cb),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0) {
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

    void restart(DateTime now);

private:
    const TimerCallback callback_;
    DateTime expiration_;
    const double interval_;
    const bool repeat_;
};

} // namespace tinyrpc

#endif /* TIMER */
