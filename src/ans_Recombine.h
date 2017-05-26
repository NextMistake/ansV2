//
// Created by well on 4/28/17.
//

#ifndef ANSV2_ANS_RECOMBINE_H
#define ANSV2_ANS_RECOMBINE_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

using std::vector;
using std::queue;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::function;
using std::make_shared;

class thread_pool {
public:
    thread_pool(int thread_num);

    ~thread_pool();

    bool start();

    bool stop();

    bool add_job(function<void()> &&f);

private:
    vector<thread> _workers;
    queue<function<void()>> _tasks;

    mutex _queue_mutex;
    condition_variable _thread_condition;

    bool run;
    int _thread_num;
};

class ans_session {
    std::vector<function> fps;
};

class ans_Recombine {

public:
    ans_Recombine();
    ~ans_Recombine();
    bool read_file();
    bool flush();
};

thread_pool::thread_pool(int thread_num):
        _thread_num(thread_num),
        run(false){
}

bool thread_pool::start() {
    if (this->run) { return false; }

    for (int i = 0; i < this->_thread_num; i++) {
        this->_workers.emplace_back(
                [this]() {
                    while(true) {
                        std::function<void()> task;
                        {
                            unique_lock<mutex> lock(this->_queue_mutex);

                            this->_thread_condition.wait(lock, [this]() {
                                return !this->run || !this->_tasks.empty();
                            });

                            if (!this->run || this->_tasks.empty()) {
                                return;
                            }

                            task = std::move(this->_tasks.front());
                            this->_tasks.pop();
                        }

                        task();
                    }
                }
        );
    }
}

bool thread_pool::stop() {
    if (!this->run) {
        return false;
    }

    {
        unique_lock<mutex> lock(_queue_mutex);
        this->run = false;
    }

    this->_thread_condition.notify_all();

    for (thread &worker : this->_workers) {
        worker.join();
    }

    this->_workers.clear();
}

bool thread_pool::add_job(function<void()> &&task) {
    {
        unique_lock<mutex> lock(this->_queue_mutex);

        this->_tasks.emplace(task);
    }

    this->_thread_condition.notify_one();

    return true;
}

thread_pool::~thread_pool() {}

#endif //ANSV2_ANS_RECOMBINE_H
