//
// Created by well on 4/28/17.
//

#include <fstream>

#include "ans_Config.h"

#include "nlohann_json.hpp"

ans_Config::ans_Config() {}

bool ans_Config::read_config(std::string) {
    std::ifstream json_file("config.json");
    if (!json_file) {
        std::cout << "open config.json error" << std::endl;
        return false;
    }

    nlohmann::json json = nlohmann::json::parse(json_file);
    json_file.close();

    // flow path config reading
    _flowroot = (json["file"]["rootpath"] != nullptr) ? json["file"]["rootpath"].get<std::string>() : nullptr;

    _resumable = json["file"]["resumable"].get<bool>()? true : false;

    _persist = json["file"]["persist"].get<bool>() ? true : false;

    if (nullptr != json["file"]["matcher"]) {
        for (nlohmann::json::iterator ite = json["file"]["matcher"].begin();
             ite != json["file"]["matcher"].end(); ite++) {
            _matchers.push_back(ite.value().get<std::string>());
        }
    }

    // pkt filters config reading
    if (nullptr != json["pkt_filters"]) {
        for (nlohmann::json::iterator ite = json["pkt_filters"].begin();
                ite != json["pkt_filters"].end(); ite++) {
            _filters.push_back(ite.value().get<std::string>());
        }
    }

    // run config reading
    _max_memory = (json["run"]["max_memory"] != nullptr) ? json["run"]["max_memory"].get<int>() : -1;

    _max_thread = (json["run"]["max_thread"] != nullptr) ? json["run"]["max_thread"].get<int>() : -1;

    _daemon = json["run"]["daemon"].get<std::string>() == "yes" ? true : false;

    // plugin config reading
    if (nullptr != json["plugins"]) {
        for (nlohmann::json::iterator ite = json["plugins"].begin();
                ite != json["plugins"].end(); ite++) {
            _plugins.push_back(ite.value().get<std::string>());
        }
    }

    // db config reading
    _db_host = (json["db"]["host"] != nullptr) ? json["db"]["host"].get<std::string>() : nullptr;
    _db_user = (json["db"]["user"] != nullptr) ? json["db"]["user"].get<std::string>() : nullptr;
    _db_passwd = (json["db"]["passwd"] != nullptr) ? json["db"]["passwd"].get<std::string>() : nullptr;
    _db_name = (json["db"]["name"] != nullptr) ? json["db"]["name"].get<std::string>() : nullptr;
    _db_port = (json["db"]["port"] != nullptr) ? json["db"]["port"].get<int>() : 3306;

    // log config reading
    _logfile = (json["logfile"] != nullptr) ? json["logfile"].get<std::string>() : nullptr;

    return true;
}

bool ans_Config::is_resumable() const {
    return _resumable;
}

void ans_Config::set_resumable(bool _resumable) {
    ans_Config::_resumable = _resumable;
}

bool ans_Config::is_persist() const {
    return _persist;
}

void ans_Config::set_persist(bool _persist) {
    ans_Config::_persist = _persist;
}

const std::string &ans_Config::get_flowroot() const {
    return _flowroot;
}

void ans_Config::set_flowroot(const std::string &_flowroot) {
    ans_Config::_flowroot = _flowroot;
}

const std::vector<std::string> &ans_Config::get_matchers() const {
    return _matchers;
}

void ans_Config::set_matchers(const std::vector<std::string> &_matchers) {
    ans_Config::_matchers = _matchers;
}

const std::vector<std::string> &ans_Config::get_filters() const {
    return _filters;
}

void ans_Config::set_filters(const std::vector<std::string> &_filters) {
    ans_Config::_filters = _filters;
}

int ans_Config::get_max_memory() const {
    return _max_memory;
}

void ans_Config::set_max_memory(int _max_memory) {
    ans_Config::_max_memory = _max_memory;
}

int ans_Config::get_max_thread() const {
    return _max_thread;
}

void ans_Config::set_max_thread(int _max_thread) {
    ans_Config::_max_thread = _max_thread;
}

bool ans_Config::is_daemon() const {
    return _daemon;
}

void ans_Config::set_daemon(bool _daemon) {
    ans_Config::_daemon = _daemon;
}

const std::vector<std::string> &ans_Config::get_plugins() const {
    return _plugins;
}

void ans_Config::set_plugins(const std::vector<std::string> &_plugins) {
    ans_Config::_plugins = _plugins;
}

const std::string &ans_Config::get_db_host() const {
    return _db_host;
}

void ans_Config::set_db_host(const std::string &_db_host) {
    ans_Config::_db_host = _db_host;
}

const std::string &ans_Config::get_db_user() const {
    return _db_user;
}

void ans_Config::set_db_user(const std::string &_db_user) {
    ans_Config::_db_user = _db_user;
}

const std::string &ans_Config::get_db_passwd() const {
    return _db_passwd;
}

void ans_Config::set_db_passwd(const std::string &_db_passwd) {
    ans_Config::_db_passwd = _db_passwd;
}

const std::string &ans_Config::get_db_name() const {
    return _db_name;
}

void ans_Config::set_db_name(const std::string &_db_name) {
    ans_Config::_db_name = _db_name;
}

int ans_Config::get_db_port() const {
    return _db_port;
}

void ans_Config::set_db_port(int _db_port) {
    ans_Config::_db_port = _db_port;
}

const std::string &ans_Config::get_logfile() const {
    return _logfile;
}

void ans_Config::set_logfile(const std::string &_logfile) {
    ans_Config::_logfile = _logfile;
}
