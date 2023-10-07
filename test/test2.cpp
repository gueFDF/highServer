#include "EventLoop.h"
#include "Thread.h"

highServer::EventLoop* g_loop;

void threadFunc() {
    g_loop->loop();
}

int main() {
    highServer::EventLoop loop;
    g_loop = &loop;
    highServer::Thread t(threadFunc);
    t.start();
    t.join();
}
