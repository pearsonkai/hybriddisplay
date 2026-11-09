#ifndef POOL_HPP
#define POOL_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace hybriddisplay::threading {

using Task = std::function<void()>;

class Pool {
private:
    std::queue<Task> tasks;
    std::vector<std::thread> threads;

    std::mutex queueMutex;
    std::condition_variable condition;
    std::condition_variable completionCondition;

    bool paused;
    bool stopping;
    size_t activeTasks;

    void work(); // Default worker loop for threads.
public:
    Pool(size_t threadCount = std::max((unsigned int) 1, std::thread::hardware_concurrency() - 1));
    ~Pool();

    uint8_t getNumThreads();
    
    /* 
        Adds a task to the queue. The task should be a callable object (function, lambda, etc.) that takes no arguments and returns void.
        Functions with arguments can be wrapped in a lambda to capture the arguments, e.g.:
        
        `pool->add([&](){ foo(variable); });`

        IMPORTANT: Don't add tasks that wait for completion of tasks from the same pool - this will cause a deadlock
    */
    void add(Task task);

    void clear(); // Clear all queued tasks without stopping the pool.

    void stopWeak(); // Stops the pool after all tasks are completed.
    void stopHard(); // Stops the pool immediately, discarding all tasks in the queue.
    void wait(); // Blocks until all tasks complete.
    bool completed(); // Returns true if all tasks are complete and the queue is empty.

    void pause(); // Temporarily pause new work while allowing currently running tasks to finish.
    void resume(); // Resume processing queued work.
};

}
#endif