#include "AsyncLogging.h"
#include "LogFile.h"
#include "Thread.h"
#include <cassert>
#include <functional>

namespace highServer {

__thread AsyncLogging* g_asyncLog = nullptr;

AsyncLogging::AsyncLogging(
    const std::string& basename, off_t rollSize, int flushInterval) :
    flushInterval_(flushInterval),
    basename_(basename), rollSize_(rollSize), thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"), cond_(), currentBuffer_(new Buffer), nextBuffer_(new Buffer), buffers_() {
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
    g_asyncLog = this;
}

void AsyncLogging::start() {
    assert(g_asyncLog != nullptr);
    auto output = [](const char* logline, int len) {
        g_asyncLog->append(logline, len);
    };

    auto flush = []() {
        g_asyncLog->stop();
    };
    Logger::setOutput(output);
    Logger::setFlush(flush);
    running_ = true;
    thread_.start();
}

void AsyncLogging::append(const char* logline, int len) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (currentBuffer_->avail() > len) {
        currentBuffer_->append(logline, len);
    } else {
        buffers_.push_back(std::move(currentBuffer_));

        if (nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);
        } else {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);
        cond_.notify();
    }
}

void AsyncLogging::threadFunc() {
    // 只有一个线程用来写入,不需要线程安全
    LogFile output(basename_, rollSize_, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    bool first = true;
    while (running_ || first) {
        // 临界区
        {
            first = false;
            std::lock_guard<std::mutex> lock(mutex_);
            if (buffers_.empty()) {
                // 等待唤醒
                cond_.waitForSeconds(flushInterval_);
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_) { nextBuffer_ = std::move(newBuffer2); }
        }

        for (const auto& buffer : buffersToWrite) {
            output.append(buffer->data(), buffer->length());
        }

        // buffersToWrite只留两个buffer(后面用)
        if (buffersToWrite.size() > 2) { buffersToWrite.resize(2); }

        if (!newBuffer1) {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2) {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        output.flush();
    }
}

void AsyncLogging::StartAsyncLog() {
}

}; // namespace highServer