#ifndef THREAD_SAFE_MESSAGE_QUEUE_H
#define THREAD_SAFE_MESSAGE_QUEUE_H

#include <queue>
#include <functional>
#include <mutex>

using Task = std::function<void()>;

class ThreadSafeMessageQueue
{
private:
    std::queue<Task> messageQueue;
    std::mutex mutex;

public:
    ThreadSafeMessageQueue();
    ~ThreadSafeMessageQueue();

    void push(Task task);
    bool pop(Task& task);
};

#endif
