#ifndef LOGFILE
#define LOGFILE
#include <memory>
#include <string>
#include "AppendFile.h"
#include "noncopyable.h"
#include <mutex>
namespace tinyrpc {

class LogFile : noncopyable {
public:
    LogFile(const std::string& basename,
            off_t rollSize,
            bool threadSafe = true,
            int flushInterval = 3,   // 默认刷新间隔为3
            int checkEveryN = 1024); // 默认检查间隔为1024
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();

private:
    void append_unlocked(const char* logline, int len);

    // 获取日志文件名
    static std::string getLogFileName(const std::string& basename, time_t* now);

    const std::string basename_; // 日志文件名，默认保存在当前工作目录下
    const off_t rollSize_;       // 日志文件超过设定值进行roll
    const int flushInterval_;    // flush时间间隔
    const int checkEveryN_;      // 多少次日志操作，检查是否刷新，是否roll

    int count_; // 对当前日志文件，进行的日志操作次数，结合checkEveryN_使用

    std::unique_ptr<std::mutex> mutex_; // 操作Appendfiles是否加锁
    time_t startOfPeriod_;              // 用于标记同一天的时间戳
    time_t lastRoll_;                   // 上一次roll的时间戳
    time_t lastFlush_;                  // 上一次flush的时间戳
    std::unique_ptr<AppendFile> file_;
    const static int kRollPerSeconds_ = 60 * 60 * 24; // 一天的秒数
};

} // namespace tinyrpc

#endif /* LOGFILE */
