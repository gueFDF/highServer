#ifndef TIMERID
#define TIMERID
namespace tinyrpc {
class Timer;
class TimerId {
public:
    explicit TimerId(Timer* timer) :
        value_(timer) {
    }

private:
    Timer* value_;
};

} // namespace tinyrpc

#endif /* TIMERID */
