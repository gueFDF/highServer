#include "Logging.h"
#include "DateTime.h"
#include "CurrentThread.h"

namespace tinyrpc {

__thread char t_errnobuf[512]; // 记录错误信息
__thread char t_time[64];      // 当前线程的时间字符串“年：月：日：时：分：秒”
__thread time_t t_lastSecond;  // 当前线程上一次日志记录时的秒数

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
    {
        " [TRACE] ",
        " [DEBUG] ",
        " [INFO]  ",
        " [WARN]  ",
        " [ERROR] ",
        " [FATAL] ",
};

const char* strerror_tl(int savedErrno) {
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

// Impl的构造函数
Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file, int line) :
    time_(GetTimeNow()), // 登记当前时间
    stream_(),           // Logstream类
    level_(level),       // 日志级别
    line_(line),         // 行号
    basename_(file)      // 文件名称
{
    getTime(); // 将时间输出到buffer中
    tid();     // 缓存当前线程tid

    // 将当前线程id和日志等级名称输出到buf当中
    stream_ << t_cachedTid;
    stream_ << LogLevelName[level];
    if (savedErrno != 0) // 如果savedErrno不为0，saveErrno为错误号
    {
        // 将错误码对应的错误信息存入t_errnobuf当中，并且输入到stream_的buf当中
        stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

void Logger::Impl::getTime() {
    time_.gettime();
    stream_ << time_.toIsoString();
}

// 所有信息输出完毕，然后再将文件名称和行号输出到buf当中
void Logger::Impl::finish() {
    stream_ << " - " << basename_ << ':' << line_ << '\n';
}

// 默认为stdout
void defaultOutput(const char* msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
    // FIXME check n
    (void)n;
}

// 默认为stdout
void defaultFlush() {
    fflush(stdout);
}

// 设置默认的输出和刷新函数
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

// 初始化日至等级
Logger::LogLevel initLogLevel() {
    // getenv()用来取得参数name环境变量的内容。
    // 参数name为环境变量的名称，如果该变量存在则会返回指向该内容的指针。
    if (::getenv("ASYNLOG_TRACE"))
        return Logger::TRACE;
    else if (::getenv("ASYNLOG_DEBUG"))
        return Logger::DEBUG;
    else
        return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

void Logger::setLogLevel(Logger::LogLevel level) {
    g_logLevel = level;
}

// Logger的四个构造函数
Logger::Logger(SourceFile file, int line) :
    impl_(INFO, 0, file, line) {
}

Logger::Logger(SourceFile file, int line, LogLevel level) :
    impl_(level, 0, file, line) {
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func) :
    impl_(level, 0, file, line) {
    impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, bool toAbort) :
    impl_(toAbort ? FATAL : ERROR, errno, file, line) {
}

// 主要在析构进行
Logger::~Logger() {
    impl_.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (impl_.level_ == FATAL) {
        // 如果这是一个FATAL刷新缓冲区，然后直接中断程序
        g_flush();
        abort();
    }
}
} // namespace tinyrpc
