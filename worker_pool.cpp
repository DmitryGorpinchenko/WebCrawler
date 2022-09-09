#include "worker_pool.h"

TaskQueue::TaskQueue()
    : request_quit(false)
{
}

size_t TaskQueue::size() const
{
    std::lock_guard lock(mtx);
    return queue.size();
}

void TaskQueue::addTask(UniqueTask&& task)
{
    {
        std::lock_guard lock(mtx);
        queue.push_back(std::move(task));
    }
    cv.notify_all();
}

UniqueTask TaskQueue::removeTask()
{
    std::unique_lock lock(mtx);
    cv.wait(lock, [this]() { return (!queue.empty()) || request_quit; });
    if (!request_quit) {
        UniqueTask task(std::move(queue.front()));
        queue.pop_front();
        return task;
    }
    return {};
}

void TaskQueue::quit()
{
    {
        std::lock_guard lock(mtx);
        request_quit = true;
    }
    cv.notify_all();
}

WorkerPool::WorkerPool(size_t _pool_size, TaskQueue& _task_queue)
    : pool_size(_pool_size)
    , task_queue(_task_queue)
{
}

WorkerPool::~WorkerPool()
{
    quit();
    wait();
}

void WorkerPool::start()
{
    for (size_t i = 0; i < pool_size; ++i) {
        workers.push_back(std::thread([this]() {
            while (true) {
                if (auto task = task_queue.removeTask()) {
                    task->perform();
                } else {
                    break;
                }
            }
        }));
    }
}

void WorkerPool::quit()
{
    task_queue.quit();
}

void WorkerPool::wait()
{
    for (auto& w : workers) {
        if (w.joinable()) {
            w.join();
        }
    }
}
