
#include "DateTime.h"
#include "EventLoop.h"
#include "TimerId.h"
#include "Logging.h"
#include <stdio.h>
#include <unistd.h>

int cnt = 0;
highServer::EventLoop* g_loop;
highServer::TimerId aid;
int count = 0;
void printTid() {
    printf("pid = %d, tid = %d\n", getpid(), highServer::tid());
    printf("now %s\n", highServer::DateTime().toIsoString().c_str());
}

void print(const char* msg) {
    printf("msg %s %s\n", highServer::DateTime().toIsoString().c_str(), msg);
    if (++cnt == 20) {
        g_loop->quit();
    }
}

void printc(const char* msg) {
    printf("msg %s %s\n", highServer::DateTime().toIsoString().c_str(), msg);
    if (++count == 6) {
        g_loop->cancel(aid);
    }
}

int main() {
    // highServer::Logger::setLogLevel(highServer::Logger::TRACE);
    printTid();
    highServer::EventLoop loop;
    g_loop = &loop;

    print("main");
    loop.runAfter(1, std::bind(print, "once1"));
    loop.runAfter(1.5, std::bind(print, "once1.5"));
    loop.runAfter(2.5, std::bind(print, "once2.5"));
    loop.runAfter(3.5, std::bind(print, "once3.5"));
    aid = loop.runEvery(2, std::bind(printc, "every2"));
    loop.runEvery(3, std::bind(print, "every3"));

    loop.loop();
    print("main loop exits");
    sleep(1);
}
