//
// Created by Yair on 22/04/2019.
//

#define _32

#include "platform.h"
#include "dir.h"
#include "utils.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno> // perror(), errno
#include <unistd.h> // read(), close()
#include <sys/inotify.h> // inotify_init(), inotify_add_watch()

#include <vector>
#include <string>
#include <iostream>

#define LOG_SEPERATOR '|'

using namespace std;

typedef std::vector<InotifyWatch*> watch_list;

/**
 * A vector of file descriptors for the inotify file/directory watches
 */
watch_list watchList;
int inotifyFd = -1;

bool init() {
    inotifyFd = inotify_init();
    if (inotifyFd < 0) {
        LOGE("inotify_init() failed!\n");
        switch (errno) {
            case EMFILE:
                LOGE("The user limit on the total number of inotify instances has been reached.\n");
                LOGE("The per-process limit on the number of open file descriptors has been reached.\n");
            break;
            case ENFILE:
                LOGE("The system-wide limit on the total number of open files has been reached.\n");
            break;
            case ENOMEM:
                LOGE("Insufficient kernel memory is available.\n");
            break;
        }
        return false;
    }
    return true;
}

bool addWatch(string path, bool isWatchSubDirs) {
    // Make sure not to add watchs while processing events
    if(isRunning) pthread_mutex_lock(&threadLock);
    if(fileExists(path.c_str())) {
        InotifyWatch* inotifyWatch;
        int fd;
        if(isWatchSubDirs) {
            struct dir_t* dir = init(path.c_str());
            char* subPath;
            for(int i = 0; i < dir->size; i++) {
                subPath = dir->subDirs[i];
                fd = inotify_add_watch(inotifyFd, subPath, IN_ACCESS | IN_MODIFY | IN_CREATE | IN_DELETE);
                if(fd) {
                    inotifyWatch = createInotifyWatch(subPath, fd);
                    watchList.push_back(inotifyWatch);
                    LOGI("Added watch: %s", subPath);
                }
                else {
                    LOGE("Failed adding watch: %s", subPath);
                }
            }
            if(isRunning) pthread_mutex_unlock(&threadLock);
            return true;
        }
        else {
            fd = inotify_add_watch(inotifyFd, path.c_str(), IN_ACCESS | IN_MODIFY | IN_CREATE | IN_DELETE);
            if(fd) {
                inotifyWatch = createInotifyWatch(path, fd);
                watchList.push_back(inotifyWatch);
                LOGI("Added watch: %s", path.c_str());
                if(isRunning) pthread_mutex_unlock(&threadLock);
                return true;
            }
        }
    }
    if(isRunning) pthread_mutex_unlock(&threadLock);
    return false;
}

void start() {
    isRunning = true;
    watchThread = new pthread_t;
    pthread_create(watchThread, NULL, &watchLoop, NULL);
    pthread_mutex_init(&threadLock, NULL);
}

void stop() {
    isRunning = false;
}

void* watchLoop(void*) {
    char eventsBuffer[4096];
    char eventMsg[512];

    while (isRunning) {
        pthread_mutex_lock(&threadLock);
        ssize_t eventsBufferSize = read(inotifyFd, eventsBuffer, sizeof(eventsBuffer));
        if (eventsBufferSize < 0) {
            LOGE("failed reading.\n");
            continue;
        }

        char *buffPtr = eventsBuffer; // Beginning of the buffer
        char *buffPtrEnd = buffPtr + eventsBufferSize; // End of the events in the buffer

        while (buffPtr < buffPtrEnd) {
            struct inotify_event *event;

            // Check if we have enough space
            if ((buffPtrEnd - buffPtr) < sizeof(struct inotify_event)) {
                LOGE("Not enough room for the inotify_event event, breaking.\n");
                break;
            }

            event = (struct inotify_event *) buffPtr;
            int c;
            char *eventMsgPtr = eventMsg;
            memset(eventMsg, 0, sizeof(eventMsg));
            string* currentPath = nullptr;
            size_t currentPathSize = 0;

            for (int i = 0; i < watchList.size(); i++) {
                if (watchList[i]->fd == event->wd) {
                    currentPath = &watchList[i]->path;
                    currentPathSize = currentPath->size();
                    break;
                }
            }

            // Add the timestamp
            char timestamp[32] = {0};
            c = sprintf(timestamp, "%lld", getCurrentTimeMillis());
            char* timestampPtr = timestamp + c;
            *(timestampPtr++) = LOG_SEPERATOR;

            c = snprintf(eventMsgPtr, strlen(timestamp) + 2, "%s", timestamp);
            eventMsgPtr += c;

            if(currentPathSize + event->len > 0) {
                // Add the directory path
                char *fullPath = (char *) calloc(currentPathSize + event->len + 2, sizeof(char));
                char *fullPathPtr = fullPath;
                size_t filePathLen = 0;

                if (currentPath != nullptr) {
                    c = snprintf(fullPathPtr, currentPathSize + 2, "%s/", currentPath->c_str());
                    fullPathPtr += c;
                    filePathLen += c;
                }

                // Add the file name(is exists)
                if (event->len) {
                    c = snprintf(fullPathPtr, event->len, "%s", event->name);
                    filePathLen += c;
                }

                c = snprintf(eventMsgPtr, filePathLen + 1, "%s", fullPath);
                eventMsgPtr += c;

                free(fullPath);
            }

            // Add the event
            if (event->mask & IN_ACCESS) {
                if ((eventMsgPtr - eventMsg) > 0) *(eventMsgPtr++) = LOG_SEPERATOR;
                c = snprintf(eventMsgPtr, 7, "ACCESS");
                eventMsgPtr += c;
            }
            if (event->mask & IN_MODIFY) {
                if ((eventMsgPtr - eventMsg) > 0) *(eventMsgPtr++) = LOG_SEPERATOR;
                c = snprintf(eventMsgPtr, 7, "MODIFY");
                eventMsgPtr += c;
            }
            if (event->mask & IN_CREATE) {
                if ((eventMsgPtr - eventMsg) > 0) *(eventMsgPtr++) = LOG_SEPERATOR;
                c = snprintf(eventMsgPtr, 7, "CREATE");
                eventMsgPtr += c;
            }
            if (event->mask & IN_DELETE) {
                if ((eventMsgPtr - eventMsg) > 0) *(eventMsgPtr++) = LOG_SEPERATOR;
                c = snprintf(eventMsgPtr, 7, "DELETE");
                eventMsgPtr += c;
            }

            *(eventMsgPtr++) = LOG_SEPERATOR;

            // Add if the event is related to a dir or file
            if (event->mask & IN_ISDIR) {
                c = snprintf(eventMsgPtr, 4, "DIR");
                eventMsgPtr += c;
            } else {
                c = snprintf(eventMsgPtr, 5, "FILE");
                eventMsgPtr += c;
            }

            LOGI("%sֿ", eventMsg);

            // Move to next event
            buffPtr += sizeof(struct inotify_event) + event->len;
        }
        pthread_mutex_unlock(&threadLock);
    }
}

void closePlatform() {
    if(watchThread != nullptr) {
        pthread_join(*watchThread, nullptr);
    }

    pthread_mutex_destroy(&threadLock);

    for (auto &item : watchList) {
        inotify_rm_watch(inotifyFd, item->fd);
    }

    if(inotifyFd > 0) {
        close(inotifyFd);
        inotifyFd = -1;
    }
}