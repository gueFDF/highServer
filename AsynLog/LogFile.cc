#include "LogFile.h"
#include "cstdint"
#include "noncopyable.h"
#include <bits/types/time_t.h>
#include <cassert>
#include <mutex>
#include <unistd.h>
namespace tinyrpc {

LogFile::LogFile(const std::string& basename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN) :
    basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    count_(0),
    mutex_(threadSafe ? new std::mutex : NULL),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0) {
    assert(basename.find('/') == std::string::npos);
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len) {
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        append_unlocked(logline, len);
    } else {
        append_unlocked(logline, len);
    }
}

void LogFile::flush() {
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        file_->flush();
    } else {
        file_->flush();
    }
}

bool LogFile::rollFile() {
    time_t now = 0;

    std::string filename = getLogFileName(basename_, &now);

    // 对齐(kRollPerSeconds_,整数倍),整天数
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

void LogFile::append_unlocked(const char* logline, int len) {
    file_->append(logline, len);

    if (file_->writtenBytes() > rollSize_) {
        rollFile();
    } else {
        ++count_;
        // 检查是否需要flush
        if (count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
            // 不是同一天(跨天)
            if (thisPeriod_ != startOfPeriod_) {
                rollFile();
            } else if (now - lastFlush_ > flushInterval_) {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

// 获取日志文件名
std::string LogFile::getLogFileName(const std::string& basename, time_t* now) {
    std::string filename;
    // 预留空间
    filename.reserve(basename.size() + 64);
    filename = basename;

    // 获取当前年月日
    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm);
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
    filename += timebuf;

    // 获取主机名
    char buf[256];
    if (::gethostname(buf, sizeof(buf)) == 0) {
        buf[sizeof(buf) - 1] = '\0';
        filename += buf;
    } else {
        filename += "unknownhost";
    }

    // 获取线程id
    char pidbuf[32];
    snprintf(pidbuf, sizeof(pidbuf), ".%d", ::getpid());

    filename += pidbuf;
    filename += ".log";
    return filename;
}

} // namespace tinyrpc