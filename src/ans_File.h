//
// Created by well on 4/28/17.
//

#ifndef ANSV2_ANS_FILE_H
#define ANSV2_ANS_FILE_H

#include <string>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <map>

#include <semaphore.h>
#include <time.h>
#include <sys/inotify.h>

#include "spdlog/spdlog.h"

enum class ans_Wached_Type {
    ROOT = 0,
    TASK = 1,
    DATE = 2
};

class ans_Watched_Item {
public:
    ans_Watched_Item(int wd,
                     int father_wd,
                     int child_wd,
                     const std::string &dir_path,
                     ans_Wached_Type type,
                     const std::shared_ptr<std::vector<std::string>> &ip_list) :
            _wd(wd),
            _father_wd(father_wd),
            _child_wd(child_wd),
            _dir_path(dir_path),
            _type(type),
            _ip_list(ip_list) {}

    int get_wd() const;

    void set_wd(int _wd);

    int get_father_wd() const;

    void set_father_wd(int _father_wd);

    const std::string &get_dir_path() const;

    void set_dir_path(const std::string &_dir_path);

    ans_Wached_Type get_type() const;

    void set_type(ans_Wached_Type _type);

    int get_child_wd() const;

    void set_child_wd(int _child_wd);

    const std::shared_ptr<std::vector<std::string>> &get_ip_list() const;

    void set_ip_list(const std::shared_ptr<std::vector<std::string>> &_ip_list);

private:
    /*
     * if type is ROOT, _father_wd is set as -1, _child_wd is set as -1
     * if type is TASK, _father_wd and _child_wd is set
     * if type is DATE, _father_wd is set and _child_wd is set as -1
     *
     * a TASK filder can have more than one DATE folder at one time
     *
     * if type is ROOT, _ip_list is not set
     * if type is TASK and DATE, _ip_list is set
     */

    int _wd;

    int _father_wd;

    int _child_wd;

    std::string _dir_path; // _dir_path end without '/'

    ans_Wached_Type _type;

    std::shared_ptr<std::vector<std::string>> _ip_list;
};

class ans_Flow_File {
public:
    ans_Flow_File(time_t _last_modify_time,
                  const std::string &_task_id,
                  const std::string &_path,
                  const std::shared_ptr<std::vector<std::string>> &_ip_list);

    ~ans_Flow_File();

    time_t get_last_modify_time() const;

    const std::string &get_task_id() const;

    const std::string &get_path() const;

    const std::shared_ptr<std::vector<std::string>> &get_ip_list() const;

private:
    time_t _last_modify_time;

    std::string _task_id;

    std::string _path;

    std::shared_ptr<std::vector<std::string>> _ip_list;
};

class ans_File {
public:
    ans_File(const bool persist,
             const std::string & flow_root,
             const std::shared_ptr<spdlog::logger> &logger);

    ~ans_File();

    /*********/

    bool start();

    void stop();

    void reset();

    std::shared_ptr<ans_Flow_File> next();

    std::shared_ptr<ans_Flow_File> next(const struct timespec *abs_timeout);

    bool has_next();

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
    bool _inotify_fd_init;
    bool _monitor_thread_on;
    int _inotify_fd;

    time_t _last_modify_time;

    sem_t *_flow_file_queue_sem;

    std::string _flow_root;

    std::mutex _flow_file_queue_mutex;

    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<std::vector<std::shared_ptr<ans_Flow_File>>> _temp_vector; // sort
    std::shared_ptr<std::queue<std::shared_ptr<ans_Flow_File>>> _flow_file_queue;
    std::shared_ptr<std::map<int, std::shared_ptr<ans_Watched_Item>>> _watch_dirs;
    std::shared_ptr<std::thread> _monitor_thread;

    bool traverse();
    void monitor();

    // these four functions needs to improve to be dynamic.
    int traverse_task_dir(const char *path);
    int traverse_date_dir(const char *path,
                          std::shared_ptr<std::vector<std::string>> &ip_list,
                          const char * task_id);
    void monitor_task_dir(int father_wd, const char *path);
    void monitor_date_dir(int father_wd,
                          const char *path,
                          const std::shared_ptr<std::vector<std::string>> &ip_list);
    void handle_events();
    void handler(struct inotify_event *event);
};

#endif //ANSV2_ANS_FILE_H