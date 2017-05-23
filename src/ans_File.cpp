//
// Created by well on 5/5/17.
//

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
    _temp_vector = std::make_shared<std::vector<std::shared_ptr<ans_Flow_File>>>();
    _flow_file_queue = std::make_shared<std::queue<std::shared_ptr<ans_Flow_File>>>();
    _watch_dirs = std::make_shared<std::map<int, std::shared_ptr<ans_Watched_Item>>>();

    _monitor_thread_on = false;

    _flow_file_queue_sem = new sem_t;
    sem_init(_flow_file_queue_sem, 0, 0);

    if (_flow_root.compare(_flow_root.size() - 1, 1, "/") != 0) {
        _flow_root.append("/");
    }

#ifdef ANSV2_DEBUG
    std::cout << "debug info[INFO]: ans_File->flow_root: " << _flow_root << std::endl;
#endif

    logger->info("ans_File->flow_root: {}", _flow_root);

    std::fstream ans_brk(_flow_root + "ans_brk", std::fstream::in);
    if (ans_brk.is_open()) {
        ans_brk >> _last_modify_time;
        ans_brk.close();
    } else {
        _last_modify_time = 0;
    }
}

ans_File::~ans_File() {
    stop();

    sem_destroy(_flow_file_queue_sem);
    close(_inotify_fd);
}

bool
ans_File::start() {
    if (!traverse()) {
        return false;
    }

    std::sort(_temp_vector->begin(), _temp_vector->end(),
              [](std::shared_ptr<ans_Flow_File> &a, std::shared_ptr<ans_Flow_File> &b) {
                  return a->get_last_modify_time() < b->get_last_modify_time();
              });

    std::unique_lock<std::mutex> lock(_flow_file_queue_mutex);

    for (std::vector<std::shared_ptr<ans_Flow_File>>::iterator ite = _temp_vector->begin();
         ite != _temp_vector->end(); ite++) {
        _flow_file_queue->push(*ite);
        sem_post(_flow_file_queue_sem);
    }

    /**************************************************************/
    /***********************monitor********************************/

    if (_persist) {
        _monitor_thread = std::make_shared<std::thread>(&ans_File::monitor, this);
    }

    _temp_vector->clear();
    _temp_vector->shrink_to_fit();

    if (_monitor_thread) {
        _monitor_thread_on = true;
        return true;
    } else {
        _monitor_thread_on = false;
        return false;
    }
}

void
ans_File::stop() {
    std::cout << "ans_File::stop()" << std::endl;
    if (_monitor_thread_on) {
        _monitor_thread_on = false;
        if (_monitor_thread->joinable()) {
            _monitor_thread->join();
            _logger->info("ans_File::stop() _monitor_thread exit");
        }
    }
}

void
ans_File::reset() {

}

std::shared_ptr<ans_Flow_File> ans_File::next() {
    std::shared_ptr<ans_Flow_File> result = nullptr;
    if (sem_wait(_flow_file_queue_sem) == 0) {
        std::unique_lock<std::mutex> lock(_flow_file_queue_mutex);
        if (_flow_file_queue->size() > 0) {
            result = _flow_file_queue->front();
            _flow_file_queue->pop();
        }
    }

    return result;
}

std::shared_ptr<ans_Flow_File>
ans_File::next(const struct timespec *abs_timeout) {
    std::shared_ptr<ans_Flow_File> result = nullptr;
    if (sem_timedwait(_flow_file_queue_sem, abs_timeout) == 0) {
        std::unique_lock<std::mutex> lock(_flow_file_queue_mutex);
        if (_flow_file_queue->size() > 0) {
            result = _flow_file_queue->front();
            _flow_file_queue->pop();
        }
    }

    return result;
}

bool
ans_File::has_next() {
    if (_flow_file_queue->size() || _monitor_thread_on) {
        return true;
    } else {
        return false;
    }
}

bool
ans_File::traverse() {
    char buffer[1024];
    char flow_root[1024];

    memset(flow_root, 0, 1024);
    _flow_root.copy(flow_root, 1024);

    DIR *pDIR;
    struct dirent *entry;
    struct stat statbuf;

    if ((pDIR = opendir(flow_root)) == NULL) {
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

int
ans_File::traverse_task_dir(const char *task_dir) {
    int result = 0;
    char buffer[1024];

    DIR *pDIR;
    struct dirent *entry;
    struct stat statbuf;

    if ((pDIR = opendir(task_dir)) == NULL) {
        return -1;
    }

    std::shared_ptr<std::vector<std::string>> ip_list = nullptr;

    std::string ans_ip_path;
    if (task_dir[strlen(task_dir) - 1] != '/') {
        ans_ip_path = std::string(task_dir) + "/ans_ip";
    } else {
        ans_ip_path = std::string(task_dir) + "ans_ip";
    }

    std::fstream ans_ip(ans_ip_path);

    if (ans_ip.is_open()) {
        ip_list = std::make_shared<std::vector<std::string>>();
        while (!ans_ip.eof()) {
            std::string ip;
            ans_ip >> ip;
            ip_list->push_back(ip);
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

        traverse_date_dir(buffer, ip_list, task_dir);
        ++result;
    }

    return result;
}

int
ans_File::traverse_date_dir(const char *path,
                            std::shared_ptr<std::vector<std::string>> &ip_list,
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

        _temp_vector->push_back(
                std::make_shared<ans_Flow_File>(
                        statbuf.st_mtime,
                        task_id,
                        buffer,
                        ip_list));
    }

    return result;
}

void
ans_File::monitor_task_dir(int father_wd,
                           const char *path) {
    std::shared_ptr<std::vector<std::string>> ip_list = nullptr;

    std::fstream ans_ip(std::string(path) + "/ans_ip");
    if (!ans_ip.is_open()) {
        return;
    } else {
        ip_list = std::make_shared<std::vector<std::string>>();
        while (!ans_ip.eof()) {
            std::string ip;
            ans_ip >> ip;
            ip_list->push_back(ip);
        }
        ans_ip.close();
    }

    DIR *pDIR;
    struct dirent *entry;
    struct stat statbuf;

    if ((pDIR = opendir(path)) == NULL) {
        _logger->error("ans_File->monitor: monitor dir '{}' error!", path);
        return;
    }

    int wd = inotify_add_watch(_inotify_fd, path, IN_CREATE);
    if (wd < 0) {
        closedir(pDIR);
        _logger->error("ans_File->monitor: monitor dir '{}' error!", path);
        return;
    }

    (*_watch_dirs)[wd] = std::make_shared<ans_Watched_Item>(wd,
                                       father_wd,
                                       -1,
                                       std::string(path),
                                       ans_Wached_Type::TASK,
                                       ip_list);

    _logger->info("ans_File->monitor: monitor TASK dir '{}'", path);
#ifdef ANSV2_DEBUG
    std::cout << "ans_File->monitor: monitor TASK dir '" << path << "'" << std::endl;
#endif

    char buffer[1024];

    time_t time_tmp = 0;
    char str_tmp[1024];

    while ((entry = readdir(pDIR)) != NULL) {
        if (*(entry->d_name) == '.') {
            continue;
        }

        if (entry->d_type != DT_DIR) {
            continue;
        }

        memset(buffer, 0, 1024);
        strcpy(buffer, path);
        if (buffer[strlen(buffer) - 1] != '/') {
            strcat(buffer, "/");
        }
        strcat(buffer, entry->d_name);

        if (stat(buffer, &statbuf) == -1) {
            continue;
        }

        if (time_tmp < statbuf.st_mtime) {
            time_tmp = statbuf.st_mtime;
            memset(str_tmp, 0, 1024);
            strcpy(str_tmp, buffer);
        }
    }

    monitor_date_dir(wd, str_tmp, ip_list);
}

void
ans_File::monitor_date_dir(int father_wd,
                           const char *path,
                           const std::shared_ptr<std::vector<std::string>> &ip_list) {
    int wd = inotify_add_watch(_inotify_fd, path, IN_MOVED_TO);
    if (wd < 0) {
        _logger->error("ans_File->monitor: monitor '{}' error!", path);
        return;
    }

    (*_watch_dirs)[wd] = std::make_shared<ans_Watched_Item>(wd,
                                                            father_wd,
                                                            -1,
                                                            std::string(path),
                                                            ans_Wached_Type::DATE,
                                                            ip_list);
    (*_watch_dirs)[wd]->set_child_wd(wd);

    _logger->info("ans_File->monitor: monitor DATE dir '{}'", path);
#ifdef ANSV2_DEBUG
    std::cout << "ans_File->monitor: monitor DATE dir '" << path << "'" << std::endl;
#endif

    return;
}

void
ans_File::monitor() {
    if (_inotify_fd_init) {
        close(_inotify_fd);
        _watch_dirs->clear();
        _inotify_fd_init = false;
    }

    _inotify_fd = inotify_init();

    if (_inotify_fd < 0) {
        _logger->error("ans_File->monitor: open inotify_init error!");
        return;
    }

    _inotify_fd_init = true;

    char buffer[1024];
    char flow_root[1024];

    memset(flow_root, 0, 1024);
    _flow_root.copy(flow_root, 1024, 0);

    memset(buffer, 0, 1024);
    strncpy(buffer, flow_root, strlen(flow_root) - 1);

    int wd = inotify_add_watch(_inotify_fd, buffer, IN_CREATE);
    if (wd < 0) {
        _logger->error("ans_File->monitor: monitor dir '{}' error!", buffer);
        return;
    }

    (*(_watch_dirs))[wd] = std::make_shared<ans_Watched_Item>(
            wd,
            -1,
            -1,
            std::string(buffer),
            ans_Wached_Type::ROOT,
            nullptr
    );

    _logger->info("ans_File->monitor: monitor ROOT dir '{}'", buffer);
#ifdef ANSV2_DEBUG
    std::cout << "ans_File->monitor: monitor ROOT dir '" << buffer << "'" << std::endl;
#endif

    DIR *pDIR;
    struct dirent *dirent;

    if ((pDIR = opendir(flow_root)) == NULL) {
        _logger->error("ans_File->monitor: open dir '{}' error!", flow_root);
        close(_inotify_fd);
        _watch_dirs->clear();
        return;
    }

    while ((dirent = readdir(pDIR)) != NULL) {
        if (*(dirent->d_name) == '.') {
            continue;
        }

        if (dirent->d_type != DT_DIR) {
            continue;
        }

        memset(buffer, 0, 1024);
        strcpy(buffer, flow_root);
        if (buffer[strlen(buffer) - 1] != '/') {
            strcat(buffer, "/");
        }
        strcat(buffer, dirent->d_name);

        monitor_task_dir(wd, buffer);
    }

    handle_events();

    return;
}

void ans_File::handle_events() {
    char buffer[16384];
    size_t  event_size;
    ssize_t p_buffer,r;
    struct inotify_event *event, *p_event;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(_inotify_fd, &rfds);
    struct timeval timeout;

    while (_watch_dirs->size() && _monitor_thread_on) {
        timeout.tv_sec = 0;
        timeout.tv_usec = 50;

        if (select(FD_SETSIZE, &rfds, NULL, NULL, &timeout) > 0) {
            r = read(_inotify_fd, buffer, 16384);
            if (r < 0) {
                continue;
            }

            p_buffer = 0;

            while (p_buffer < r) {
                p_event = (struct inotify_event*) &buffer[p_buffer];
                event_size = offsetof(struct inotify_event, name) + p_event->len;

                event = (struct inotify_event *)malloc(event_size);
                memmove(event, p_event, event_size);

                handler(event);

                free(event);
                p_buffer += event_size;
            }
        }
    }

    for (auto ite = _watch_dirs->begin(); ite != _watch_dirs->end(); ite++) {
        int wd = ite->first;
        inotify_rm_watch(_inotify_fd, wd);
    }

    _watch_dirs->clear();
    close(_inotify_fd);
    _inotify_fd = -1;
    _inotify_fd_init = false;

    return;
}

void
ans_File::handler(struct inotify_event *event) {
    struct stat statbuf;

    std::shared_ptr<ans_Watched_Item> item = (*_watch_dirs)[event->wd];
    std::string str_path = item->get_dir_path() + "/" + event->name;

    char c_path[1024];

    memset(c_path, 0, 1024);
    str_path.copy(c_path, 1024, 0);

    if ((stat(c_path, &statbuf)) < 0) {
        _logger->error("ans_File->monitor: stat file '{}' error!", str_path);
        return;
    }

    std::shared_ptr<ans_Watched_Item> father_item = (*_watch_dirs)[item->get_father_wd()];

    if ((event->mask & IN_CREATE) && (event->mask & IN_ISDIR)) {
        if (!S_ISDIR(statbuf.st_mode)) {
            _logger->info("ans_File->monitor: '{}' isn't a folder", str_path);
            return;
        }

        if (item->get_type() == ans_Wached_Type::ROOT) {
            monitor_task_dir(father_item->get_wd(), c_path);

        } else if (item->get_type() == ans_Wached_Type::TASK) {
            if (!father_item) {
                _logger->error("ans_File->monitor: father_item null");
                return;
            }

            if (father_item->get_child_wd() < 0) {
                _logger->error("ans_File->monitor: child_wd is '{}'", father_item->get_child_wd());
            } else {
                inotify_rm_watch(_inotify_fd, father_item->get_child_wd());
                _watch_dirs->erase(father_item->get_child_wd());
            }

            int wd = inotify_add_watch(_inotify_fd, c_path, IN_CREATE);
            if (wd < 0) {
                father_item->set_child_wd(-1);
                _logger->error("ans_File->monitor: monitor dir '{}' error!");
                return;
            }

            father_item->set_child_wd(wd);

            (*_watch_dirs)[wd] = std::make_shared<ans_Watched_Item>(wd,
                                                                    father_item->get_wd(),
                                                                    -1,
                                                                    str_path,
                                                                    ans_Wached_Type::DATE,
                                                                    father_item->get_ip_list());
        }
    } else if (event->mask & IN_MOVED_TO) {
        if (!S_ISREG(statbuf.st_mode)) {
            _logger->info("ans_File->monitor: '{}' isn't a reg file!");
            return;
        }

        if (strcmp(event->name - 5, ".pcap") != 0) {
            _logger->info("ans_File->monitor: '{}' isn't a pcap file!");
            return;
        }

        std::shared_ptr<ans_Flow_File> file =
                std::make_shared<ans_Flow_File>(statbuf.st_mtime,
                                                father_item->get_dir_path(),
                                                str_path,
                                                father_item->get_ip_list());
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

ans_Flow_File::~ans_Flow_File() {

}

time_t ans_Flow_File::get_last_modify_time() const {
    return _last_modify_time;
}

const std::string &ans_Flow_File::get_task_id() const {
    return _task_id;
}

const std::string &ans_Flow_File::get_path() const {
    return _path;
}

const std::shared_ptr<std::vector<std::string>> &ans_Flow_File::get_ip_list() const {
    return _ip_list;
}

ans_Flow_File::ans_Flow_File(time_t _last_modify_time, const std::string &_task_id, const std::string &_path,
                             const std::shared_ptr<std::vector<std::string>> &_ip_list) : _last_modify_time(
        _last_modify_time), _task_id(_task_id), _path(_path), _ip_list(_ip_list) {}

int ans_Watched_Item::get_wd() const {
    return _wd;
}

void ans_Watched_Item::set_wd(int _wd) {
    ans_Watched_Item::_wd = _wd;
}

int ans_Watched_Item::get_father_wd() const {
    return ans_Watched_Item::_father_wd;
}

void ans_Watched_Item::set_father_wd(int _father_wd) {
    ans_Watched_Item::_father_wd = _father_wd;
}

const std::string &ans_Watched_Item::get_dir_path() const {
    return ans_Watched_Item::_dir_path;
}

void ans_Watched_Item::set_dir_path(const std::string &_dir_path) {
    ans_Watched_Item::_dir_path = _dir_path;
}

ans_Wached_Type ans_Watched_Item::get_type() const {
    return ans_Watched_Item::_type;
}

void ans_Watched_Item::set_type(ans_Wached_Type _type) {
    ans_Watched_Item::_type = _type;
}

int ans_Watched_Item::get_child_wd() const {
    return _child_wd;
}

void ans_Watched_Item::set_child_wd(int _child_wd) {
    ans_Watched_Item::_child_wd = _child_wd;
}

const std::shared_ptr<std::vector<std::string>> &ans_Watched_Item::get_ip_list() const {
    return _ip_list;
}

void ans_Watched_Item::set_ip_list(const std::shared_ptr<std::vector<std::string>> &_ip_list) {
    ans_Watched_Item::_ip_list = _ip_list;
}
