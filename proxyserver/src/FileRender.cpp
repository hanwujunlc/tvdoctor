/*
 * FileRender.cpp
 *
 *  Created on: Dec 31, 2015
 *      Author: hwj
 */

#include "FileRender.h"
#include "sys_def.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

FileRender::FileRender(int sockfd, std::string filename, int rw_type, int file_type) :
	m_filename(filename),
	m_source_id(0),
	m_target_id(0),
	m_sockfd(sockfd),
	m_file_size(0),
	m_data_size(0),
	m_rw_type(rw_type),
	m_file_type(file_type)
{
	if(0 == m_rw_type) {
		std::string file_path = FILE_PATH + m_filename;
		m_filefd = open(file_path.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	} else if(1 == m_rw_type) {
		std::string file_path = FILE_PATH + m_filename;
		m_filefd = open(file_path.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	}
}

FileRender::~FileRender() {
	close(m_filefd);
}


int FileRender::writeToFile(unsigned char *buf, unsigned int len)
{
	if(!buf) return -1;

	int ret = write(this->m_filefd, buf, len);

	//LOG_DEBUG("write to file %d data size %u, write size %d", this->m_filefd, len, ret);
	return ret;
}

int FileRender::readFromFile(unsigned char *buf, unsigned int len)
{
	return read(this->m_filefd, buf, len);
}

int FileRender::writeMsgToTV(int sockfd, unsigned char *buf, unsigned int len)
{
	int ret = write(sockfd, buf, len);
	//LOG_DEBUG("write to TV %d data size %u, write size %d", sockfd, len, ret);
	return ret;
}

int FileRender::writeMsg(int sockfd, unsigned char *buf, unsigned int len)
{
	int ret = write(sockfd, buf, len);
	//LOG_DEBUG("write to %d data size %u, write size %d", sockfd, len, ret);
	return ret;
}

int FileRender::writeFileToTV(int sockfd)
{
	//TODO
//	char buf[BUFF_SIZE];
//	int ret = 0;
//	while ((ret = read(this->m_filefd, buf, BUFF_SIZE)) > 0) {
//		 write(sockfd, buf, ret);
//	}
	return 0;
}

unsigned long FileRender::getFileSize() {
	unsigned long filesize = 0;
	struct stat statbuff;
	std::string file_path = FILE_PATH + m_filename;
	if (stat(file_path.c_str(), &statbuff) < 0) {
		return filesize;
	} else {
		filesize = statbuff.st_size;
	}
	return filesize;
}
