#include <iostream>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <assert.h>
#include <time.h>
#include "thread_pool.h"

class ThreadPool {
private:
    std::vector<std::thread> _threads;
    TaskQueue* myque;
    std::mutex mtx;
    std::condition_variable cond;
    bool shutdown = false;
public:
    ThreadPool(int number = 5) : _threads(std::vector<std::thread> (number)),
        myque(new TaskQueue()) {
        
    }
    ~ThreadPool() {
        shutdown = true;
    }
    
    void init() {
        for(int i = 0; i < _threads.size(); ++i) {
            _threads[i]  = std::thread(&ThreadPool::run, this);
            _threads[i].detach();
        }
    }

    void* run() {
        std::function<void()> func;
        bool flag = false;
        while(!shutdown) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                if(myque->is_empty()) {
                    cond.wait(lock);
                }
                flag = true;
                func = myque->getTask();
            }   
            if(flag) {
                func();
            }

        }
    }
    template<typename F, typename... Args>
    auto commit(F &f, Args &&...args) -> decltype( myque->addTask(f, std::forward<Args> (args)...)) {
        auto ret = myque->addTask(f, std::forward<Args> (args)...);
        cond.notify_one();
        return ret;
    }
};

std::mutex m_mtx;

int func(int x) {
    auto now = std::time(nullptr);
    auto date_time = localtime(&now);
    std::unique_lock<std::mutex> lock(m_mtx);
    std::cout << "任务编号：" << x <<" 执行线程ID: " 
        << std::this_thread::get_id() << " 当前时间: " 
        << date_time->tm_min << ":" << date_time->tm_sec << std::endl;
    lock.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return x;
}

int main() {
    ThreadPool thread_pool(5);
    thread_pool.init();
    for(int i = 0; i < 20; ++i) {
        auto ret = thread_pool.commit(func, i);
        int rec = ret.get();
    }
    std::this_thread::sleep_for(std::chrono::seconds(20));
    return 0;
}
