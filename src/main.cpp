#include <iostream>

#include <signal.h>

#include "ans.h"

std::function<void()> fp;

void stop(int) {
    if (fp) {
        fp();
    }
    exit(0);
}

int main() {

    signal(SIGINT, stop);

    ans_Config config;
    config.read_config("config.json");
    Ans ans{config};

    fp = std::bind(&Ans::stop, &ans);

    ans.run();

    return 0;
}