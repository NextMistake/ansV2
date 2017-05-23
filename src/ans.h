//
// Created by well on 5/3/17.
//

#ifndef ANSV2_ANS_H
#define ANSV2_ANS_H

#include <vector>
#include <memory>

#include <semaphore.h>

#include "ans_Config.h"
#include "ans_File.h"

#include "spdlog/spdlog.h"

class Ans {
public:
    Ans(const ans_Config &config);

    virtual ~Ans();

    void run();

    void stop();

    const ans_Config &getConfig() const;

    void setConfig(const ans_Config &config);

private:
    ans_Config _config;

    std::shared_ptr<spdlog::logger> _logger;

    std::shared_ptr<ans_File> _file_module;
};

#endif //ANSV2_ANS_H