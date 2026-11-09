#ifndef POOL_HPP
#define POOL_HPP

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

    bool stopping;
    size_t activeTasks;
public:
    Pool(size_t threadCount = );
    ~Pool();

    uint8_t getNumThreads();
    
    // IMPORTANT: Don't add tasks that wait for completion of all tasks - this will cause a deadlock
    void add(Task task); // pool->add([&](){ foo(variable); });
    
    void requestStop();
    void wait();
    bool completed();

    void work();
};

}
#endif