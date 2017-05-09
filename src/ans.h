//
// Created by well on 5/3/17.
//

#ifndef ANSV2_ANS_H
#define ANSV2_ANS_H

#include <vector>
#include <memory>

#include "ans_Config.h"
#include "ans_File.h"

#include "spdlog/spdlog.h"

class Ans {
public:
    Ans(const ans_Config &config);

    void run();
private:
    bool monitor;
    bool breakpoint;


    std::shared_ptr<spdlog::logger> logger;
    std::unique_ptr<ans_File> ans_file;
};

#endif //ANSV2_ANS_H