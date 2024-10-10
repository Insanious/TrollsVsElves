#include "ThreadSafeMessageQueue.h"

ThreadSafeMessageQueue::ThreadSafeMessageQueue() {}

ThreadSafeMessageQueue::~ThreadSafeMessageQueue() {}

void ThreadSafeMessageQueue::push(Task task)
{
    std::lock_guard<std::mutex> lock(mutex);
    messageQueue.push(task);
}

bool ThreadSafeMessageQueue::pop(Task& task)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (messageQueue.empty())
        return false;

    task = messageQueue.front();
    messageQueue.pop();
    return true;
}
