//
// Created by well on 5/3/17.
//

#include <iostream>

#include "ans.h"

Ans::Ans(const ans_Config &config) {
    try {
        logger = spdlog::daily_logger_mt("daily_logger", config.get_logfile(), 0, 0);
    } catch (spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }

    logger->set_pattern("[%H:%M:%S %z] [p %p] [t %t] [%l] %v");
//    // Release and close all loggers
//    spdlog::drop_all();

}