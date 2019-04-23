//
// Created by Yair on 22/04/2019.
//

#ifndef UTILS_H
#define UTILS_H

#include <cstdio>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <cstring>
#include <cstdarg>
#include <android/log.h>

// Android log function wrappers
static const char* kTAG = "StalkerLib";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

long long getCurrentTimeMillis();
bool fileExists(const char* filename);
void getCurrentUser(char *buf, size_t size);

#endif
