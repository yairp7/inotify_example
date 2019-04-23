//
// Created by Yair on 22/04/2019.
//

#ifndef PLATFORM_H
#define PLATFORM_H

#include <cstdio>
#include <string>
#include <pthread.h>

using namespace std;

struct inotify_watch {
    public:
        string path;
        uint32_t fd = 0;
};

typedef struct inotify_watch InotifyWatch;

static InotifyWatch* createInotifyWatch(string path, uint32_t fd) {
    auto inotifyWatch = new InotifyWatch;
    inotifyWatch->path = path;
    inotifyWatch->fd = fd;
    return inotifyWatch;
}

static pthread_t* watchThread;
static pthread_mutex_t threadLock;
static bool isRunning = false;

bool init();
bool addWatch(string path, bool isWatchSubDirs);
bool initLib();
void start();
void stop();
void closePlatform();
void* watchLoop(void*);

#endif