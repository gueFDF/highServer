#include "Logging.h"
#include "AsyncLogging.h"
#include "Thread.h"
#include <unistd.h>

using namespace highServer;
int main() {
    highServer::AsyncLogging log;

    log.start();
    Logger::setLogLevel(Logger::DEBUG);
    LOG_INFO << "12345678910";
    LOG_DEBUG << 12345678910;
    LOG_ERROR << "abcdefghijklmnopqrstuvwxyzABCDEF";
    LOG_INFO << 1.123466;
    LOG_INFO << 12.3456789;
}
