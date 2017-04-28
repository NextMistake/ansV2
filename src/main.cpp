#include <iostream>

#include "ans_Config.h"

int main() {
    ans_Config config;
    config.read_config("config.json");
    std::cout << "Hello, World!" << std::endl;
    return 0;
}