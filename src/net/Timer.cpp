#include "Timer.h"
#include "DateTime.h"

using namespace tinyrpc;

void Timer::restart(DateTime now) {
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        expiration_.invalid();
    }
}