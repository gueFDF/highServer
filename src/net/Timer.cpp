#include "Timer.h"
#include "DateTime.h"
#include "TimerQueue.h"

using namespace tinyrpc;

std::atomic_int64_t Timer::s_numCreated_;
void Timer::restart(DateTime now) {
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        expiration_.invalid();
    }
}