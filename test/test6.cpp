#include "EventLoop.h"
#include "EventLoopThread.h"
#include <stdio.h>
#include <unistd.h>
#include <TimerId.h>
void runInThread() {
    printf("runInThread(): pid = %d, tid = %d\n",
           getpid(), tinyrpc::tid());
}

int main() {
    printf("main(): pid = %d, tid = %d\n",
           getpid(), tinyrpc::tid());

    tinyrpc::EventLoopThread loopThread;
    tinyrpc::EventLoop* loop = loopThread.startLoop();
    loop->runInLoop(runInThread);
    sleep(1);
    loop->runAfter(2, runInThread);
    sleep(3);
    loop->quit();

    printf("exit main().\n");
}
