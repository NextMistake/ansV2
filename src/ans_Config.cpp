//
// Created by well on 4/28/17.
//

#include <fstream>

#include "ans_Config.h"

#include "nlohann_json.hpp"

ans_Config::ans_Config() {}

ans_Config::ans_Config(const ans_Config &config):
_persist(config._persist),
_flow_root(config._flow_root),
_filter(config._filter),
_plugins(config._plugins),
_log_file(config._log_file)
{}

bool ans_Config::read_config(std::string) {
    std::ifstream json_file("config.json");
    if (!json_file) {
        std::cout << "open config.json error" << std::endl;
        return false;
    }

    nlohmann::json json = nlohmann::json::parse(json_file);
    json_file.close();

    _persist = json["persist"].get<bool>() ? true : false;

    _flow_root = (json["root_path"] != nullptr) ? json["root_path"].get<std::string>() : nullptr;

    _filter = (json["pkt_filter"] != nullptr) ? json["pkt_filter"].get<std::string>() : nullptr;

    if (nullptr != json["plugins"]) {
        for (nlohmann::json::iterator ite = json["plugins"].begin();
                ite != json["plugins"].end(); ite++) {
            _plugins.push_back(ite.value().get<std::string>());
        }
    }

    _log_file = (json["log_file"] != nullptr) ? json["log_file"].get<std::string>() : nullptr;

    return true;
}

bool ans_Config::is_persist() const {
    return _persist;
}

void ans_Config::set_persist(bool _persist) {
    ans_Config::_persist = _persist;
}

const std::string &ans_Config::get_flow_root() const {
    return _flow_root;
}

void ans_Config::set_flow_root(const std::string &_flow_root) {
    ans_Config::_flow_root = _flow_root;
}

const std::string &ans_Config::get_filter() const {
    return _filter;
}

void ans_Config::set_filter(const std::string &_filter) {
    ans_Config::_filter = _filter;
}

const std::vector<std::string> &ans_Config::get_plugins() const {
    return _plugins;
}

void ans_Config::set_plugins(const std::vector<std::string> &_plugins) {
    ans_Config::_plugins = _plugins;
}

const std::string &ans_Config::get_log_file() const {
    return _log_file;
}

void ans_Config::set_log_file(const std::string &_log_file) {
    ans_Config::_log_file = _log_file;
}
