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
    if (nullptr != json["file"]["rootpath"]) {
        flowroot = json["file"]["rootpath"];
    } else {
        flowroot = nullptr;
    }

    if (nullptr != json["file"]["breakpoint"]) {
        if (json["file"]["breakpoint"].get<std::string>() == "on") {
            breakpoint = true;
        } else {
            breakpoint = false;
        }
    } else {
        breakpoint = false;
    }

    if (nullptr != json["file"]["monitor"]) {
        if (json["file"]["monitor"].get<std::string>() == "on") {
            monitor = true;
        } else {
            monitor = false;
        }
    } else {
        monitor = false;
    }

    if (nullptr != json["file"]["matcher"]) {
        for (nlohmann::json::iterator ite = json["file"]["matcher"].begin();
             ite != json["file"]["matcher"].end(); ite++) {
            matchers.push_back(ite.value().get<std::string>());
        }
    }

    // pkt filters config reading
    if (nullptr != json["pkt_filters"]) {
        for (nlohmann::json::iterator ite = json["pkt_filters"].begin();
                ite != json["pkt_filters"].end(); ite++) {
            filters.push_back(ite.value().get<std::string>());
        }
    }

    // run config reading
    if (nullptr != json["run"]["max_memory"]) {
        max_memory = json["run"]["max_memory"];
    } else {
        max_memory = -1;
    }

    if (nullptr != json["run"]["max_thread"]) {
        max_thread = json["run"]["max_thread"];
    } else {
        max_thread = -1;
    }

    // plugin config reading
    if (nullptr != json["plugins"]) {
        for (nlohmann::json::iterator ite = json["plugins"].begin();
                ite != json["plugins"].end(); ite++) {
            plugins.push_back(ite.value().get<std::string>());
        }
    }

    // db config reading
    if (nullptr != json["db"]["host"] &&
        nullptr != json["db"]["user"] &&
        nullptr != json["db"]["passwd"] &&
        nullptr != json["db"]["name"]) {
        db_host = json["db"]["host"];
        db_user = json["db"]["user"];
        db_passwd = json["db"]["passwd"];
        db_name = json["db"]["name"];
    } else {
        db_host = db_user = db_passwd = db_name = nullptr;
    }

    if (nullptr != json["db"]["port"]) {
        db_port = json["db"]["port"];
    } else {
        db_port = 3306;
    }

    // log config reading
    if (nullptr != json["log"]["log_root"]) {
        logroot = json["log"]["log_root"];
    } else {
        logroot = "logs/";
    }

    return true;
}