#ifndef __SYS_DEF__
#define __SYS_DEF__

#include <stdio.h>

#define __DEBUG__

#ifdef __DEBUG__
#define DEBUG(format,...) printf("File: "__FILE__", Line: %03d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif


#define MAX_EVENTS 10
#define BUFF_SIZE 1024 * 10
#define PHP_TIMEOUT 65

#define WEBSOCK_PORT 9008
#define TCPSOCK_PORT 9001
#define PHPSOCK_PORT 9002
#define FILESOCK_PORT 9003
#define LOGSOCK_PORT 9004
#define LOGWEBSOCK_PORT 9005
#define SERVER_ID 0000
//#define FILE_PATH "/var/www/"
#define FILE_PATH "/usr/share/nginx/html/"

#define DEFAULT_LOG_PATH "/var/log/"
#define DEFAULT_LOG_FILE "tvdoctor.log"
#define DEFAULT_LOG_MAXSIZE 1024 * 10

#define LOG_DEBUG_LEVEL 4

#define EPOLL_TIMEOUT 100

#endif
