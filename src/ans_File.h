//
// Created by well on 4/28/17.
//

#ifndef ANSV2_ANS_FILE_H
#define ANSV2_ANS_FILE_H

#include <string>
#include <queue>
#include <memory>

#include <semaphore.h>

class ans_File_Breakpoint {
public:
    ans_File_Breakpoint();
};

class ans_File_Flow {
public:
    ans_File_Flow();
    ans_File_Flow(ans_File_Flow &file);
    ans_File_Flow(ans_File_Flow &&file);
private:
    std::string path;
    const std::shared_ptr<std::vector<std::string>> ip;
};

class ans_File {
public:
    ans_File();

    void run();
private:
    std::string _flowroot;
    bool resumable;
    bool persist;

    std::queue<ans_File_Flow>& _filequeue;

    sem_t *_filesem;

    bool traverse(const char *path);
};

#endif //ANSV2_ANS_FILE_H