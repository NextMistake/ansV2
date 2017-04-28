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
    std::string flow_root;
    std::vector<ans_PathMatcher> path_matchers;
    bool monitor;

    // db configls
    std::string db_host;
    std::string db_user;
    std::string db_passwd;
    std::string db_db;
    int db_port;

    // log config
    ans_Log log;

    // run config
    int max_memory/*MB*/;
    int max_thread;

    ans_File file_module;

};

#endif //ANSV2_ANS_CONFIG_H
