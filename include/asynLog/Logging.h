#ifndef LOGGING
#define LOGGING
#include "SoruceFile.h"
#include "DateTime.h"
#include "LogStream.h"

namespace highServer {

class Logger {
public:
    // 日志等级
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    LogStream& stream() {
        return impl_.stream_;
    }

    static LogLevel logLevel();              // 获取日志等级
    static void setLogLevel(LogLevel level); // 设置日志等级

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();
    static void setOutput(OutputFunc); // 设置输出刷新函数
    static void setFlush(FlushFunc);   // 设置刷新函数

private:
    class Impl {
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void getTime();       // 将时间输出到buffer中
        void finish();        // 一条日志的结束
        DateTime time_;       // 日志时间戳
        LogStream stream_;    // 日志缓存流
        LogLevel level_;      // 日志级别
        int line_;            // 当前记录日志宏的源代码行号
        SourceFile basename_; // 当前记录日志宏的源代码名称
    };
    Impl impl_;
};

// 全局日志等级
extern Logger::LogLevel g_logLevel;
inline Logger::LogLevel Logger::logLevel() {
    return g_logLevel;
}

// 日志宏
#define LOG_TRACE                                                    \
    if (highServer::Logger::logLevel() <= highServer::Logger::TRACE) \
    highServer::Logger(__FILE__, __LINE__, highServer::Logger::TRACE, __func__).stream()
#define LOG_DEBUG                                                    \
    if (highServer::Logger::logLevel() <= highServer::Logger::DEBUG) \
    highServer::Logger(__FILE__, __LINE__, highServer::Logger::DEBUG, __func__).stream()
#define LOG_INFO                                                    \
    if (highServer::Logger::logLevel() <= highServer::Logger::INFO) \
    highServer::Logger(__FILE__, __LINE__).stream()

#define LOG_WARN highServer::Logger(__FILE__, __LINE__, highServer::Logger::WARN).stream()
#define LOG_ERROR highServer::Logger(__FILE__, __LINE__, highServer::Logger::ERROR).stream()
#define LOG_FATAL highServer::Logger(__FILE__, __LINE__, highServer::Logger::FATAL).stream()
#define LOG_SYSERR highServer::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL highServer::Logger(__FILE__, __LINE__, true).stream()

} // namespace highServer
#endif /* LOGGING */
