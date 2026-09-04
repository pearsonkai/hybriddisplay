#include "Pool.hpp"

namespace hybriddisplay::threading {

Pool::Pool(size_t threadCount) : stopping(false), activeTasks(0) {
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
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopping = true;
        std::queue<Task>().swap(tasks);
    }

    condition.notify_all();

    for (std::thread& worker : threads)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
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
                return stopping || !tasks.empty();
            });

            if (stopping && tasks.empty())
            {
                return;
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

void Pool::addTask(Task task) {
    std::lock_guard<std::mutex> lock(queueMutex);

    if (stopping) {
        return;
    }

    tasks.push(std::move(task));

    condition.notify_one();
}

void Pool::waitForCompletion() {
    std::unique_lock<std::mutex> lock(queueMutex);
    completionCondition.wait(lock, [this]
    {
        return tasks.empty() && activeTasks == 0;
    });
}

}