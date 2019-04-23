//
// Created by Yair on 22/04/2019.
//

#include "dir.h"

void reset() {
    if(_current != NULL) {
        for(int i = 0; i < _current->size; i++) {
            free(_current->subDirs[i]);
        }
        free(_current->subDirs);
        free(_current);
    }
}

struct dir_t* init(const char* rootPath) {
    if(_current != NULL) {
        reset();
    }

    _current = (struct dir_t*)calloc(1, sizeof(struct dir_t));
    _current->size = 0;
    _current->capacity = DEFAULT_NUM_OF_DIRS;
    _current->subDirs = (char**)calloc(_current->capacity, sizeof(char*));

    listDir(rootPath, addDir);

    return _current;
}

static void addDir(char* dirPath) {
    if(_current != NULL) {
        if(_current->size >= _current->capacity * 0.8) {
            int newCapacity = _current->capacity * 2;
            char** newSubDirs = (char**)calloc(newCapacity, sizeof(char*));
            _current->capacity = newCapacity;
            for(int i = 0; i < _current->size; i++) {
                newSubDirs[i] = _current->subDirs[i];
            }
            free(_current->subDirs);
            _current->subDirs = newSubDirs;
        }

        char* dirPathCopy = (char*)calloc(strlen(dirPath) + 1, sizeof(char));
        strcpy(dirPathCopy, dirPath);
        _current->subDirs[_current->size] = dirPathCopy;
        _current->size++;
    }
}

struct dir_t* get() {
    return _current;
}

static void listDir(const char* root, void (*addDir)(char* dirPath)) {
    DIR *dir;
    struct dirent *entry;

    LOGI("Listing dirs in %s.\n", root);

    if (!(dir = opendir(root))) {
        return;
    }

    size_t rootLen = strlen(root);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            char *format;
            if (strcmp(root, ".") == 0 || root[rootLen - 1] != '/') {
                snprintf(path, sizeof(path), "%s/%s", root, entry->d_name);
            } else {
                snprintf(path, sizeof(path), "%s%s", root, entry->d_name);
            }

            LOGI("Directory found: \"%s\".\n", path);

            addDir(path);
            listDir(path, addDir);
        }
    }
    closedir(dir);
}