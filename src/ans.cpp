//
// Created by well on 5/3/17.
//

#include <iostream>

#include "ans.h"

Ans::Ans(const ans_Config &config): _config(config) {
    try {
        _logger = spdlog::daily_logger_mt("daily_logger", _config.get_log_file(), 0, 0);
    } catch (spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }

    _logger->set_pattern("[%H:%M:%S %z] [p %p] [t %t] [%l] %v");
    // Release and close all loggers
//    spdlog::drop_all();

    _file_module = std::make_shared<ans_File>(_config.is_persist(), _config.get_flow_root(), _logger);
}

Ans::~Ans() {}

void Ans::run() {
    _file_module->start();

    while (true) {
        std::cout << _file_module->next()->get_path() << std::endl;
    }
}

void Ans::stop() {
    _logger->info("Ans::stop()");

    _file_module->stop();

    _logger->flush();
}

const ans_Config &Ans::getConfig() const {
    return _config;
}

void Ans::setConfig(const ans_Config &config) {
    Ans::_config = config;
}

