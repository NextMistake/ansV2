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

private:

    // flow path config
    bool breakpoint;
    bool monitor;
    std::string flowroot;
    std::vector<std::string> matchers;

    // filters config
    std::vector<std::string> filters;

    // run config
    int max_memory/*MB*/;
    int max_thread;

    // plugins config
    std::vector<std::string> plugins;

    // db config
    std::string db_host;
    std::string db_user;
    std::string db_passwd;
    std::string db_name;
    int db_port;

    // log config
    std::string logroot;
};

#endif //ANSV2_ANS_CONFIG_H
