#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>
#include "Logging.h"
#include <strings.h>
#include <stdio.h>
#include <sys/timerfd.h>

tinyrpc::EventLoop* g_loop;

void timeout() {
    printf("Timeout!\n");
    g_loop->quit();
}

int main() {
    tinyrpc::Logger::setLogLevel(tinyrpc::Logger::TRACE);
    tinyrpc::EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    tinyrpc::Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);

    loop.loop();

    ::close(timerfd);
}
