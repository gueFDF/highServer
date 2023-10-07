#include "EventLoop.h"
#include "Logging.h"
#include <cstdio>
#include <unistd.h>
void threadFunc() {
    printf("threadFunc(): pid = %d, tid = %d\n", getpid(), highServer::tid());
    highServer::EventLoop loop;
    loop.loop();
}

int main() {
    highServer::Logger::setLogLevel(highServer::Logger::TRACE);
    printf("main(): pid = %d, tid = %d\n", getpid(), highServer::tid());
    highServer::Thread thread(threadFunc);
    thread.start();
    highServer::EventLoop loop;
    loop.loop();
}