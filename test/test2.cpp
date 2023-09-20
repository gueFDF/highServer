#include "EventLoop.h"
#include "Thread.h"

tinyrpc::EventLoop* g_loop;

void threadFunc() {
    g_loop->loop();
}

int main() {
    tinyrpc::EventLoop loop;
    g_loop = &loop;
    tinyrpc::Thread t(threadFunc);
    t.start();
    t.join();
}
