//
// Created by well on 4/28/17.
//

#include <fstream>

#include "ans_Config.h"

#include "nlohann_json.hpp"

ans_Config::ans_Config() {}

bool ans_Config::read_config(std::string) {
    std::ifstream json_file("config.json");

    if (nullptr == json_file) {
        printf("error");
        return false;
    }

    nlohmann::json json = nlohmann::json::parse(json_file);

    json_file.close();
    std::cout << json << std::endl;

    std::cout << json["db"] << std::endl;

    return true;
}