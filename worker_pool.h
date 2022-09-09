#ifndef WORKER_POOL_H
#define WORKER_POOL_H

#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>

struct ITask {
    virtual ~ITask() = default;
    virtual void perform() = 0;
};
using UniqueTask = std::unique_ptr<ITask>;

class TaskQueue {
public:
    TaskQueue();

    size_t size() const;

    void addTask(UniqueTask&& task);
    UniqueTask removeTask();

    void quit();

private:
    bool request_quit;
    std::condition_variable cv;
    mutable std::mutex mtx;
    std::deque<std::unique_ptr<ITask>> queue;
};

class WorkerPool {
public:
    explicit WorkerPool(size_t _pool_size, TaskQueue& _task_queue);
    ~WorkerPool();

    void start();
    void quit();
    void wait();

private:
    const size_t pool_size;
    TaskQueue& task_queue;
    std::vector<std::thread> workers;
};

#endif
