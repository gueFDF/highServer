#include "DateTime.h"

#include <sys/time.h>

namespace tinyrpc {

std::string DateTime::toIsoString() const {
    char buf[64];
    snprintf(buf, sizeof buf, "%04d-%02d-%02d %02d:%02d:%02d",
             year, month, day, hour, minute, second);
    return buf;
}

DateTime::DateTime(const struct tm& t) :
    year(t.tm_year + 1900), month(t.tm_mon + 1), day(t.tm_mday),
    hour(t.tm_hour), minute(t.tm_min), second(t.tm_sec) {
}

void DateTime::gettime() {
    time_t nowtime;
    struct tm* t;
    time(&nowtime);
    t = localtime(&nowtime);
    year = t->tm_year + 1900;
    month = t->tm_mon + 1;
    day = t->tm_mday;
    hour = t->tm_hour;
    minute = t->tm_min;
    second = t->tm_sec;
}
DateTime GetTimeNow() {
    time_t nowtime;
    struct tm* timeinfo;
    time(&nowtime);
    timeinfo = localtime(&nowtime);

    return DateTime(*timeinfo);
}
} // namespace DateTime