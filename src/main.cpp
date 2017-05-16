#include <iostream>

#include "ans.h"
#include "ans_Config.h"

int main() {
    ans_Config config;
    config.read_config("config.json");
    Ans ans{config};
    ans.run();
    std::cout << "main return 0" << std::endl;
    return 0;
}