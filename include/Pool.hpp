#ifndef POOL_HPP
#define POOL_HPP

#include <vector>
#include <thread>
#include <queue>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace hybriddisplay::threading {

/*

IMPORTANT IMPLIMENTATION DISCLAIMER:



*/


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
    Pool(size_t threadCount);
    ~Pool();

    // IMPORTANT: Don't add tasks that wait for completion of all tasks - this will cause a deadlock
    void addTask(Task task); // pool->addTask([&](){ foo(variable); });
    
    
    void requestStop();
    void waitForCompletion();
    void work();
};

}
#endif