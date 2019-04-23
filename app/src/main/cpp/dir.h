//
// Created by Yair on 22/04/2019.
//

#ifndef STALKER_DIR_H
#define STALKER_DIR_H

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

#include "utils.h"

#define DEFAULT_NUM_OF_DIRS 10

static struct dir_t {
    char** subDirs;
    int size;
    int capacity;
} dir_t;

static struct dir_t* _current;

struct dir_t* init(const char* rootPath);
struct dir_t* get();
void reset();

static void listDir(const char* root, void (*addDir)(char* dirPath));
static void addDir(char* dirPath);

#endif
