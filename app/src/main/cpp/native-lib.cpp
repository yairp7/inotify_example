//
// Created by Yair on 22/04/2019.
//

#include <jni.h>
#include <string>
#include <pthread.h>
#include <sys/inotify.h>
#include "platform.h"
#include "utils.h"

using namespace std;

extern "C" JNIEXPORT jboolean JNICALL Java_com_pech_tools_inotify_1example_MainActivity_initLib(
        JNIEnv* env,
        jobject /* this */) {
    bool result = init();
    if(!result) {
        LOGE("start() Failed.");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL Java_com_pech_tools_inotify_1example_MainActivity_start(
        JNIEnv* env,
        jobject /* this */) {
    start();
    char user[32];
    getCurrentUser(user, 32);
    LOGI("Stalker started[running as %s].", user);
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_pech_tools_inotify_1example_MainActivity_addWatch(
        JNIEnv* env,
        jobject /* this */,
        jstring path,
        jboolean isWatchSubDirs) {
    jboolean isCopy;
    const char* tmpPath = (env)->GetStringUTFChars(path, &isCopy);
    string cPath(tmpPath);
    if(!addWatch(cPath, isWatchSubDirs)) {
        LOGE("addWatch() Failed.");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL Java_com_pech_tools_inotify_1example_MainActivity_close(
        JNIEnv* env,
        jobject /* this */) {
    closePlatform();
}

