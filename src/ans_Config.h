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

    ans_Config(const ans_Config &config);

    bool read_config(std::string);

    bool is_persist() const;

    void set_persist(bool _persist);

    const std::string &get_flow_root() const;

    void set_flow_root(const std::string &_flow_root);

    const std::string &get_filter() const;

    void set_filter(const std::string &_filter);

    const std::vector<std::string> &get_plugins() const;

    void set_plugins(const std::vector<std::string> &_plugins);

    const std::string &get_log_file() const;

    void set_log_file(const std::string &_log_file);

private:
    bool _persist;

    std::string _flow_root;

    std::string _filter;

    std::vector<std::string> _plugins;

    std::string _log_file;
};

#endif //ANSV2_ANS_CONFIG_H
