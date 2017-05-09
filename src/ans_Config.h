//
// Created by well on 4/28/17.
//

#ifndef ANSV2_ANS_CONFIG_H
#define ANSV2_ANS_CONFIG_H

#include <string>
#include <vector>

#include "spdlog/spdlog.h"

#include "ans_File.h"
#include "ans_Log.h"

class ans_Config {
public:
    ans_Config();
    bool read_config(std::string);

    bool is_resumable() const;

    void set_resumable(bool _resumable);

    bool is_persist() const;

    void set_persist(bool _persist);

    const std::string &get_flowroot() const;

    void set_flowroot(const std::string &_flowroot);

    const std::vector<std::string> &get_matchers() const;

    void set_matchers(const std::vector<std::string> &_matchers);

    const std::vector<std::string> &get_filters() const;

    void set_filters(const std::vector<std::string> &_filters);

    int get_max_memory() const;

    void set_max_memory(int _max_memory);

    int get_max_thread() const;

    void set_max_thread(int _max_thread);

    bool is_daemon() const;

    void set_daemon(bool _daemon);

    const std::vector<std::string> &get_plugins() const;

    void set_plugins(const std::vector<std::string> &_plugins);

    const std::string &get_db_host() const;

    void set_db_host(const std::string &_db_host);

    const std::string &get_db_user() const;

    void set_db_user(const std::string &_db_user);

    const std::string &get_db_passwd() const;

    void set_db_passwd(const std::string &_db_passwd);

    const std::string &get_db_name() const;

    void set_db_name(const std::string &_db_name);

    int get_db_port() const;

    void set_db_port(int _db_port);

    const std::string &get_logfile() const;

    void set_logfile(const std::string &_logfile);

private:
    // flow path config
    bool _resumable;
    bool _persist;
    std::string _flowroot;
    std::vector<std::string> _matchers;

    // filters config
    std::vector<std::string> _filters;

    // run config
    int _max_memory/*MB*/;
    int _max_thread;
    bool _daemon;

    // plugins config
    std::vector<std::string> _plugins;

    // db config
    std::string _db_host;
    std::string _db_user;
    std::string _db_passwd;
    std::string _db_name;
    int _db_port;

    // log config
    std::string _logfile;
};

#endif //ANSV2_ANS_CONFIG_H
