#include "EventLoop.h"
#include "Logging.h"
#include <cstdio>
#include <unistd.h>
void threadFunc() {
    printf("threadFunc(): pid = %d, tid = %d\n", getpid(), tinyrpc::tid());
    tinyrpc::EventLoop loop;
    loop.loop();
}

int main() {
    tinyrpc::Logger::setLogLevel(tinyrpc::Logger::TRACE);
    printf("main(): pid = %d, tid = %d\n", getpid(), tinyrpc::tid());
    tinyrpc::Thread thread(threadFunc);
    thread.start();
    tinyrpc::EventLoop loop;
    loop.loop();
}