#pragma once

#include <queue>
#include <functional>
#include <mutex>
#include <future>
#include <thread>
#include <iostream>

class TaskQueue {

private:
    using Task = std::function<void()>; // 任务函数
    std::queue<Task> _task;
    std::mutex mtx;
public:
    TaskQueue() {}
    template<typename F, typename... Args>
    auto addTask(F &f, Args &&...args) -> std::future<decltype( f(args...)) > {
        std::function<decltype( f(args...)) () > ret = std::bind( f, std::forward<Args> (args)...);
        auto new_task = std::make_shared<std::packaged_task< decltype(f(args...))()>> (ret);
        
        
        std::function<void()> mytask = [new_task] {
            (*new_task) ();
        };
        std::unique_lock<std::mutex> lock(mtx);
        _task.push(mytask);
        return new_task->get_future();
    } 

    bool is_empty() {
        return _task.empty();
    }

    Task getTask() {
        Task task;
        std::unique_lock<std::mutex> lock(mtx);
        if(!_task.empty()) {
            task = std::move(_task.front());
            _task.pop();
            return task;
        }
        return nullptr;
    }

};