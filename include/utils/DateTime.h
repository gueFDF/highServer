#ifndef TIMEZONE
#define TIMEZONE
#include <bits/types/time_t.h>
#include <cstdint>
#include <string>

#include <ctime>

namespace tinyrpc {
struct date {
    int year = 0;   // [1900, 2500]
    int month = 0;  // [1, 12]
    int day = 0;    // [1, 31]
    int hour = 0;   // [0, 23]
    int minute = 0; // [0, 59]
    int second = 0; // [0, 59]
};

struct DateTime {
public:
    DateTime();
    DateTime(uint64_t t);
    uint64_t gettime();
    void update();
    void invalid(); // 使其无效
    // "2011-12-31 12:34:56"
    std::string toIsoString() const;
    bool valid() const {
        return nowtime > 0;
    }

private:
    date parse(uint64_t nanosecond) const;
    uint64_t nowtime;
};

DateTime addTime(DateTime timestamp, double seconds);

inline bool operator<(DateTime lhs, DateTime rhs) {
    return lhs.gettime() < rhs.gettime();
}

inline bool operator==(DateTime lhs, DateTime rhs) {
    return lhs.gettime() == rhs.gettime();
}

inline bool operator>(DateTime lhs, DateTime rhs) {
    return lhs.gettime() > rhs.gettime();
}
} // namespace tinyrpc
#endif /* TIMEZONE */
