//
// Created by well on 4/28/17.
//

#ifndef ANSV2_ANS_FILE_H
#define ANSV2_ANS_FILE_H

#include <string>
#include <queue>
#include <memory>
#include <mutex>

#include <semaphore.h>
#include <time.h>

#include "spdlog/spdlog.h"

class ans_Flow_File {
public:
    ans_Flow_File(const time_t &last_modify_time,
                  const std::string &task_id,
                  const std::string &path,
                  const std::vector<std::string> &ip_list):
            _last_modify_time(last_modify_time),
            _task_id(task_id),
            _path(path),
            _ip_list(ip_list){}

    ~ans_Flow_File();

    const time_t &get_last_modify_time() const;

    void set_last_modify_time(const time_t &_last_modify_time);

    const std::string &get_task_id() const;

    void set_task_id(const std::string &_task_id);

    const std::string &get_path() const;

    void set_path(const std::string &_path);

    const std::vector<std::string> &get_ip_list() const;

    void set_ip_list(const std::vector<std::string> &_ip_list);

private:
    time_t _last_modify_time;

    std::string _task_id;

    std::string _path;

    std::vector<std::string> _ip_list;
};

class ans_File {
public:
    ans_File(const bool persist,
             const std::string & flow_root,
             const std::shared_ptr<spdlog::logger> &logger);

//    ans_File(ans_File & file);
//
//    ans_File(ans_File && file);
//
//    ans_File & operator= (ans_File & file);
//
//    ans_File & operator= (ans_File && file);

    ~ans_File();

    /*********/

    bool start();

    void stop();

    void clear();

    std::shared_ptr<ans_Flow_File> next();

    /*********/

    bool is_persist() const;

    void set_persist(bool _persist);

    time_t get_last_modify_time() const;

    void set_last_modify_time(time_t _last_modify_time);

    const std::string &get_flow_root() const;

    void set_flow_root(const std::string &_flow_root);

    const std::shared_ptr<spdlog::logger> &get_logger() const;

    void set_logger(const std::shared_ptr<spdlog::logger> &_logger);

private:
    bool _persist;
    std::string _flow_root;
    std::shared_ptr<spdlog::logger> _logger;

    time_t _last_modify_time;

    std::vector<std::shared_ptr<ans_Flow_File>> _temp_vector; // sort
    std::queue<std::shared_ptr<ans_Flow_File>> _flow_file_queue;
    std::mutex _flow_file_queue_mutex;
    sem_t *_flow_file_queue_sem;

    std::shared_ptr<std::thread> _monitor_thread;

    bool traverse();
    void monitor();

    // these four functions needs to improve to be dynamic.
    int traverse_task_dir(const char *path);
    int traverse_date_dir(const char *path,
                          std::vector<std::string> &iplist,
                          const char * task_id);
    void monitor_task_dir();
    void monitor_date_dir();
};

#endif //ANSV2_ANS_FILE_H