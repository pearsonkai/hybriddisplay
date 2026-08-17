#include "Pool.hpp"

Pool::Pool(size_t threadCount) {
    stopping = false;
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
        }

        task();
    }
}

void Pool::addTask(Task task) {
    std::lock_guard<std::mutex> lock(queueMutex);

    tasks.push(std::move(task));

    condition.notify_one();
}

void Pool::waitForCompletion() {
    // Implementation for waiting for task completion
}