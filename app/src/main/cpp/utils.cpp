//
// Created by Yair on 22/04/2019.
//

//
// Created by Yair Pecherer on 04/04/2019.
//
#include "utils.h"

long long getCurrentTimeMillis() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec*1000LL + tv.tv_usec/1000;
}

bool fileExists(const char* filename) {
    return access(filename, F_OK) != -1;
}

void getCurrentUser(char *buf, size_t size) {
    char* user = getlogin();
    memcpy(buf, user, size);
}