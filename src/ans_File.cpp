//
// Created by well on 5/5/17.
//

#include <string>

#include <dirent.h>
#include <sys/stat.h>

#include "ans_File.h"

ans_File_Breakpoint::ans_File_Breakpoint() {

}

void ans_File::run() {
    /* traverse file */
    if (_flowroot.compare(_flowroot.length() - 2, 1, "/") != 0) {
        _flowroot.append("/");
    }

    /* if persist, go on; else add end file to the queue and quit */
}

bool ans_File::traverse(const char *path) {
    // C style because of Linux API

    struct stat stat;
    if (stat(path, &stat) == -1) {
        return false;
    }

    struct dirent *dirent;
    DIR *Dir;

}