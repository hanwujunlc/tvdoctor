/*
 * main.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: hwj
 */

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "ProcessManager.h"
#include "sys_def.h"
#include "logger.h"

static void init_daemon(const char *dir)
{
	int pid = fork();
	if (pid)
		exit(0);
	else if (pid < 0)
		exit(1);

	setsid();
	pid = fork();
	if (pid)
		exit(0);
	else if (pid < 0)
		exit(1);

	umask(0);

	int fd = open("/dev/null", O_RDWR);
	if (fd == -1) {
		LOG_DEBUG("open(\"/dev/null\") failed");
		return;
	}

	if (dup2(fd, STDIN_FILENO) == -1) {
		LOG_DEBUG("dup2(STDIN) failed");
		return;
	}

	if (dup2(fd, STDOUT_FILENO) == -1) {
		LOG_DEBUG("dup2(STDOUT) failed");
		return;
	}
	if (fd > STDERR_FILENO) {
		if (close(fd) == -1) {
			LOG_DEBUG("close() failed");
			return;
		}
	}
}

int main(int argc, char *argv[])
{
	//init_daemon(NULL);
	ProcessManager *instance = ProcessManager::GetInstance();
	instance->processs();
	return 0;
}


