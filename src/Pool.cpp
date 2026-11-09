#include "Pool.hpp"

namespace hybriddisplay::threading {

Pool::Pool(size_t threadCount) : paused(false), stopping(false), activeTasks(0) {
    if (threadCount == 0) {
        threadCount = 1;
    }

    for (size_t i = 0; i < threadCount; ++i)
    {
        threads.emplace_back(
            &Pool::work,
            this);
    }
}

Pool::~Pool()
{
    stopWeak();

    for (std::thread& worker : threads)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

uint8_t Pool::getNumThreads() {
    return threads.size();
}

void Pool::clear() {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::queue<Task>().swap(tasks);
    completionCondition.notify_all();
}

void Pool::stopWeak()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopping = true;
    }

    condition.notify_all();
}

void Pool::stopHard()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopping = true;
        std::queue<Task>().swap(tasks);
    }

    completionCondition.notify_all();
    condition.notify_all();
}

void Pool::work()
{
    while (true)
    {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);

            condition.wait(lock, [&]
            {
                return stopping || (!paused && !tasks.empty());
            });

            if (stopping && tasks.empty())
            {
                return;
            }

            if (paused)
            {
                continue;
            }

            if (tasks.empty())
            {
                continue;
            }

            task = std::move(tasks.front());
            tasks.pop();
            ++activeTasks;
        }

        task();

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            --activeTasks;
            if (tasks.empty() && activeTasks == 0) {
                completionCondition.notify_all();
            }
        }
    }
}

void Pool::add(Task task) {
    std::lock_guard<std::mutex> lock(queueMutex);

    if (stopping) {
        return;
    }

    tasks.push(std::move(task));
    completionCondition.notify_all();
    condition.notify_one();
}

void Pool::wait() {
    std::unique_lock<std::mutex> lock(queueMutex);
    completionCondition.wait(lock, [this]
    {
        return completed();
    });
}

bool Pool::completed() {
    return tasks.empty() && activeTasks == 0;
}

void Pool::pause() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        paused = true;
    }

    condition.notify_all();
}

void Pool::resume() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        paused = false;
    }

    condition.notify_all();
}

}