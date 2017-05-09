#include <iostream>

#include "ans.h"
#include "ans_Config.h"

int main() {
    ans_Config config;
    config.read_config("config.json");
    Ans ans{config};
    std::cout << "Hello, World!" << std::endl;
    return 0;
}