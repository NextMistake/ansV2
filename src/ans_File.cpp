//
// Created by well on 5/5/17.
//

#include <thread>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>

#include <dirent.h>
#include <sys/stat.h>

#include "ans_File.h"

ans_File::ans_File(const bool persist, const std::string &flow_root, const std::shared_ptr<spdlog::logger> &logger):
_persist(persist),
_flow_root(flow_root),
_logger(logger)
{
    _flow_file_queue_sem = new sem_t;
    sem_init(_flow_file_queue_sem, 0, 0);

    if (_flow_root.compare(_flow_root.size() - 1, 1, "/") != 0) {
        _flow_root.append("/");
    }

#ifdef ANSV2_DEBUG
    std::cout << "debug info[INFO]: ans_File->flow_root: " << _flow_root << std::endl;
#endif

    logger->info("ans_File->flow_root: {}", _flow_root);

    std::fstream breakpoint_file(_flow_root + "ans_brk", std::fstream::in);
    if (breakpoint_file.is_open()) {
        breakpoint_file >> _last_modify_time;
        breakpoint_file.close();
    } else {
        _last_modify_time = 0;
    }
}

ans_File::~ans_File() {
    if (_persist) {
        if (_monitor_thread && _monitor_thread->joinable()) {
            _monitor_thread->join();
        }
    }


    sem_destroy(_flow_file_queue_sem);
}

bool ans_File::start() {
    if (!traverse()) {
        return false;
    }

    std::sort(_temp_vector.begin(), _temp_vector.end(),
              [](std::shared_ptr<ans_Flow_File> &a, std::shared_ptr<ans_Flow_File> &b){
                  return a->get_last_modify_time() < b->get_last_modify_time();
              });

    std::unique_lock<std::mutex> lock(_flow_file_queue_mutex);

    for (std::vector<std::shared_ptr<ans_Flow_File>>::iterator ite = _temp_vector.begin();
         ite != _temp_vector.end(); ite++) {
//        std::cout << (*ite)->get_path() << std::endl;
        _flow_file_queue.push(*ite);
        sem_post(_flow_file_queue_sem);
    }

    lock.unlock();

    if (_persist) {
        _monitor_thread = std::make_shared<std::thread>(&ans_File::monitor, this);
    }

    if (_monitor_thread) {
        return true;
    } else {
        return false;
    }
}

void ans_File::stop() {

}

void ans_File::clear() {

}

std::shared_ptr<ans_Flow_File> ans_File::next() {
    std::shared_ptr<ans_Flow_File> result = nullptr;
    if (sem_wait(_flow_file_queue_sem) == 0) {
        std::unique_lock<std::mutex> lock(_flow_file_queue_mutex);
        if (_flow_file_queue.size() > 0) {
            result = _flow_file_queue.front();
            _flow_file_queue.pop();
        }
    }

    return result;
}

bool ans_File::traverse() {
    char buffer[1024];
    const char *flow_root = this->_flow_root.c_str();

    DIR *pDIR;
    struct dirent *entry;
    struct stat statbuf;

    if ((pDIR = opendir(_flow_root.data())) == NULL) {
#ifdef ANSV2_DEBUG
        std::cout << "debug info[ERROR]: ans_File->traverse: open dir'" << _flow_root
                  << "' error" << std::endl;
#endif
        _logger->error("ans_File->traverse: open dir '{}' error", _flow_root);
        return false;
    }

    while ((entry = readdir(pDIR)) != NULL) {
        if (*(entry->d_name) == '.') {
            continue;
        }

        if (entry->d_type != DT_DIR) {
            continue;
        }

        memset(buffer, 0, 1024);
        strcpy(buffer, flow_root);
        strcat(buffer, entry->d_name);

        if (stat(buffer, &statbuf) == -1) {
            continue;
        }

        if (statbuf.st_mtime < _last_modify_time) {
            continue;
        }

        traverse_task_dir(buffer);
    }

    return true;
}

int ans_File::traverse_task_dir(const char *task_dir) {
    int result = 0;
    char buffer[1024];

    DIR *pDIR;
    struct dirent *entry;
    struct stat statbuf;

    if ((pDIR = opendir(task_dir)) == NULL) {
        return -1;
    }

    std::vector<std::string> iplist;

    std::string ans_ip_path;
    if (task_dir[strlen(task_dir) - 1] != '/') {
        ans_ip_path = std::string(task_dir) + "/ans_ip";
    } else {
        ans_ip_path = std::string(task_dir) + "ans_ip";
    }

    std::fstream ans_ip(ans_ip_path);

    if (ans_ip.is_open()) {
        while (!ans_ip.eof()) {
            std::string ip;
            ans_ip >> ip;
            iplist.push_back(ip);
        }
        ans_ip.close();

    } else {
        return -1;
    }

    while ((entry = readdir(pDIR)) != NULL) {
        if (*(entry->d_name) == '.') {
            continue;
        }

        if (entry->d_type != DT_DIR) {
            continue;
        }

        memset(buffer, 0, 1024);
        strcpy(buffer, task_dir);
        if (task_dir[strlen(task_dir) - 1] != '/') {
            strcat(buffer, "/");
        }
        strcat(buffer, entry->d_name);

        if (stat(buffer, &statbuf) == -1) {
            continue;
        }

        if (statbuf.st_mtime < _last_modify_time) {
            continue;
        }

        traverse_date_dir(buffer, iplist, task_dir);
        ++result;
    }

    return result;
}

int
ans_File::traverse_date_dir(const char *path,
                            std::vector<std::string> &iplist,
                            const char *task_id) {
    int result = 0;
    char buffer[1024];

    DIR *pDIR;
    struct dirent *entry;
    struct stat statbuf;

    if ((pDIR = opendir(path)) == NULL) {
        return -1;
    }

    while ((entry = readdir(pDIR)) != NULL) {
        if (*(entry->d_name) == '.') {
            continue;
        }

        if (entry->d_type != DT_REG) {
            continue;
        }

        memset(buffer, 0, 1024);
        strcpy(buffer, path);
        if (path[strlen(path) - 1] != '/') {
            strcat(buffer, "/");
        }
        strcat(buffer, entry->d_name);

        if (stat(buffer, &statbuf) == -1) {
            continue;
        }

        if (statbuf.st_mtime < _last_modify_time) {
            continue;
        }

        if (strcmp((entry->d_name + strlen(entry->d_name) - 5), ".pcap") != 0) {
            return -1;
        }

        _temp_vector.push_back(
                std::make_shared<ans_Flow_File>(
                        statbuf.st_mtime,
                        task_id,
                        buffer,
                        iplist));
    }

    return result;
}

void ans_File::monitor_task_dir() {

}

void ans_File::monitor_date_dir() {

}

void ans_File::monitor() {
    int a = 4;
    while (a) {
        std::cout << "thraed monitor" << std:: endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        --a;
    }

}

bool ans_File::is_persist() const {
    return _persist;
}

void ans_File::set_persist(bool _persist) {
    ans_File::_persist = _persist;
}

time_t ans_File::get_last_modify_time() const {
    return _last_modify_time;
}

void ans_File::set_last_modify_time(time_t _last_modify_time) {
    ans_File::_last_modify_time = _last_modify_time;
}

const std::string &ans_File::get_flow_root() const {
    return _flow_root;
}

void ans_File::set_flow_root(const std::string &_flow_root) {
    ans_File::_flow_root = _flow_root;
}

const std::shared_ptr<spdlog::logger> &ans_File::get_logger() const {
    return _logger;
}

void ans_File::set_logger(const std::shared_ptr<spdlog::logger> &_logger) {
    ans_File::_logger = _logger;
}

/*********/
ans_Flow_File::~ans_Flow_File() {

}

const time_t &ans_Flow_File::get_last_modify_time() const {
    return _last_modify_time;
}

void ans_Flow_File::set_last_modify_time(const time_t &_last_modify_time) {
    ans_Flow_File::_last_modify_time = _last_modify_time;
}

const std::string &ans_Flow_File::get_task_id() const {
    return _task_id;
}

void ans_Flow_File::set_task_id(const std::string &_task_id) {
    ans_Flow_File::_task_id = _task_id;
}

const std::string &ans_Flow_File::get_path() const {
    return _path;
}

void ans_Flow_File::set_path(const std::string &_path) {
    ans_Flow_File::_path = _path;
}

const std::vector<std::string> &ans_Flow_File::get_ip_list() const {
    return _ip_list;
}

void ans_Flow_File::set_ip_list(const std::vector<std::string> &_ip_list) {
    ans_Flow_File::_ip_list = _ip_list;
}
