#include "DateTime.h"

#include <bits/types/time_t.h>
#include <cstdint>
#include <ctime>
#include <sys/time.h>
#include <sys/types.h>

namespace highServer {

ino64_t getnanosecond() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * (1000 * 1000) + tv.tv_usec;
}

DateTime ::DateTime() {
    nowtime = getnanosecond();
}
DateTime::DateTime(uint64_t t) {
    nowtime = t;
}

date DateTime::parse(uint64_t nanosecond) const {
    struct tm* t;
    time_t ts = static_cast<time_t>(nanosecond / (1000 * 1000));
    t = localtime(&ts);
    struct date da;
    da.year = t->tm_year + 1900;
    da.month = t->tm_mon + 1;
    da.day = t->tm_mday;
    da.hour = t->tm_hour;
    da.minute = t->tm_min;
    da.second = t->tm_sec;
    return da;
}

std::string DateTime::toIsoString() const {
    date da = parse(nowtime);
    char buf[64];
    snprintf(
        buf, sizeof buf, "%04d-%02d-%02d %02d:%02d:%02d", da.year, da.month, da.day,
        da.hour, da.minute, da.second);
    return buf;
}

uint64_t DateTime::gettime() {
    return nowtime;
}

DateTime addTime(DateTime timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * 1000 * 1000);
    return DateTime(timestamp.gettime() + delta);
}

void DateTime::update() {
    nowtime = getnanosecond();
}

void DateTime::invalid() {
    nowtime = 0;
}

} // namespace highServer