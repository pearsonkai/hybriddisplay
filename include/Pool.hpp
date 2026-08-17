#ifndef POOL_HPP
#define POOL_HPP

#include <vector>
#include <thread>
#include <queue>

namespace hybriddisplay::threading {

using Task = std::function<void()>;

class Pool {
private:
    std::queue<Task> taskQueue;
    std::vector<std::thread> threads;

    std::mutex queueMutex;
    std::condition_variable condition;

    bool stopping;
public:
    Pool(size_t numThreads);
    ~Pool();
    void addTask(Task task);
    void waitForCompletion();
};

}
#endif