#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <condition_variable>
using namespace std;

//atomic<int> goods;
int goods;
mutex mtx;
mutex com_mtx;
condition_variable cond;

void producer(string name) {
    lock_guard<mutex> guard(mtx);
    ++goods;
    cout << name << "生产者生产了一个商品，目前已有" << goods << endl;
    cond.notify_one();
    //this_thread::sleep_for(chrono::seconds(2));
}

void comsumer(string name) {
    
    unique_lock<mutex> lock(com_mtx);
    if(goods == 0) cond.wait(lock);
    
    lock_guard<mutex> guard(mtx);
    --goods;
    cout << name << "消费者消费了一个商品，目前剩余" << goods << endl;
}

int main () {
    vector<thread> A;
    vector<thread> B;

    for(int i = 0; i < 10; ++i) {
        A.push_back(thread(&producer, "A" + to_string(i)));
    }
    for(int i = 0; i < 10; ++i) {
        B.push_back(thread(&comsumer, "B" + to_string(i)));
    }
    while(1);
    // for(int i = 0; i < 100; ++i) {
    //     A[i].join();
    //     B[i].join();
    // }
    return 0;
}