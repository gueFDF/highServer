#ifndef TIMERID
#define TIMERID
#include <cstdint>
namespace highServer {
class Timer;
class TimerId {
public:
    explicit TimerId(Timer* timer = nullptr, int64_t seq = 0) :
        value_(timer),
        sequence_(seq) {
    }

private:
    friend class TimerQueue;
    Timer* value_;
    int64_t sequence_;
};

} // namespace highServer

#endif /* TIMERID */
