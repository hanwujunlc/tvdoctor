//#include <android/log.h>
#ifndef COOCAA_OS_VNC_UTILS_H_
#define COOCAA_OS_VNC_UTILS_H_

#define VNC_LOG_TAG "libcoocaavnc"
#define VNCLOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, VNC_LOG_TAG, __VA_ARGS__)
#define VNCLOGE(...)  __android_log_print(ANDROID_LOG_ERROR, VNC_LOG_TAG, __VA_ARGS__)

unsigned int crc32_hash(const unsigned char* pbData, size_t size);





#endif 
