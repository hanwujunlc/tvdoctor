/*
 * FileDataFilter.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: hwj
 */

#include "FileDataFilter.h"
#include "FileRender.h"
#include "sys_def.h"
#include "VncDataPackage.h"
#include "ConnectionManager.h"
#include "Connection.h"
#include "logger.h"

#include <string.h>

FileDataFilter::FileDataFilter(int sockfd) :
	m_cached_size(0),
	m_pkage_size(0),
	m_is_get_header(false),
	m_is_finish(false),
	m_sockfd(sockfd),
	m_filerender(NULL)
{
	// TODO Auto-generated constructor stub
	m_cache_buf = new unsigned char[BUFF_SIZE];
	m_alloc_size = BUFF_SIZE;
}

FileDataFilter::~FileDataFilter() {
	// TODO Auto-generated destructor stub
	if (m_cache_buf) {
		delete[] m_cache_buf;
		m_cache_buf = NULL;
	}
	if(m_filerender) {
		delete m_filerender;
		m_filerender = NULL;
	}
}

int FileDataFilter::addData(unsigned char *buf, unsigned int buffsize)
{
	//LOG_DEBUG("addData buf size %u", buffsize);
	if (!buf || 0 == buffsize) {
		return -1;
	}

	if (!m_is_get_header) {

		if (m_cached_size + buffsize > m_alloc_size) {
			m_alloc_size += BUFF_SIZE;
			unsigned char * newbuff = new unsigned char[m_alloc_size];
			memcpy(newbuff, m_cache_buf, m_cached_size);
			delete[] m_cache_buf;
			m_cache_buf = newbuff;
			LOG_DEBUG("addData new alloc buff size %u", m_alloc_size);
		}
		memcpy(&m_cache_buf[m_cached_size], buf, buffsize);
		m_cached_size += buffsize;
		if(m_cached_size >= 4) {
			if(0 == m_pkage_size) {
				m_pkage_size = (m_cache_buf[0] << 24) | (m_cache_buf[1] << 16) | (m_cache_buf[2] << 8) | (m_cache_buf[3]);
			}
			if(m_cached_size >= m_pkage_size && m_cached_size > 40) {
				unsigned int file_opt = (m_cache_buf[4] << 24) | (m_cache_buf[5] << 16) | (m_cache_buf[6] << 8) | (m_cache_buf[7]);
				unsigned int file_type = (m_cache_buf[8] << 24) | (m_cache_buf[9] << 16) | (m_cache_buf[10] << 8) | (m_cache_buf[11]);
				unsigned int target = (m_cache_buf[12] << 24) | (m_cache_buf[13] << 16) | (m_cache_buf[14] << 8) | (m_cache_buf[15]);
				unsigned int source = (m_cache_buf[16] << 24) | (m_cache_buf[17] << 16) | (m_cache_buf[18] << 8) | (m_cache_buf[19]);
				//unsigned int tag = (m_cache_buf[20] << 24) | (m_cache_buf[21] << 16) | (m_cache_buf[22] << 8) | (m_cache_buf[23]);
				unsigned int filesize = (m_cache_buf[24] << 24) | (m_cache_buf[25] << 16) | (m_cache_buf[26] << 8) | (m_cache_buf[27]);
				//unsigned int checksign = (m_cache_buf[28] << 24) | (m_cache_buf[29] << 16) | (m_cache_buf[30] << 8) | (m_cache_buf[31]);
				//unsigned int reserve = (m_cache_buf[32] << 24) | (m_cache_buf[33] << 16) | (m_cache_buf[34] << 8) | (m_cache_buf[35]);
				unsigned int namelen = (m_cache_buf[36] << 24) | (m_cache_buf[37] << 16) | (m_cache_buf[38] << 8) | (m_cache_buf[39]);
				char fileName[namelen + 1];
				bzero(fileName, namelen + 1);
				if (m_cached_size >= 40 + namelen) {
					memcpy(fileName, &m_cache_buf[40], namelen);
					this->m_is_get_header = true;

					//if()
//					LOG_DEBUG("file_opt = %u", file_opt);
//					LOG_DEBUG("file_type = %u", file_type);
//					LOG_DEBUG("target = %u", target);
//					LOG_DEBUG("source = %u", source);
//					LOG_DEBUG("filesize = %u", filesize);
//					LOG_DEBUG("fileName = %s", fileName);

					m_filerender = new FileRender(this->m_sockfd, std::string(fileName), file_opt, file_type);

//					m_filerender->m_rw_type = file_opt;
//					m_filerender->m_file_type = file_type;
					m_filerender->m_source_id = source;
					m_filerender->m_target_id = target;
					m_filerender->m_data_size = filesize;

					unsigned char size[5];
					bzero(size, 5);
					if (0 == m_filerender->m_rw_type) {
						memcpy(size, &m_cache_buf[24], 4);
					} else if (1 == m_filerender->m_rw_type) {
						unsigned long filesize =  m_filerender->getFileSize();
						//if (-1 != filesize)
						{
							size[0] = (filesize >> 24) & 0xff;
							size[1] = (filesize >> 16) & 0xff;
							size[2] = (filesize >> 8) & 0xff;
							size[3] = filesize & 0xff;
						}
					}
					m_filerender->writeMsgToTV(this->m_sockfd, size, 4);

					if (1 == m_filerender->m_rw_type) {
						//m_filerender->writeFileToTV(this->m_sockfd);
						m_is_finish = true;
					}
				}
			}
		}
	} else {
		if (m_filerender && 0 == m_filerender->m_rw_type) {
			m_filerender->writeToFile(buf, buffsize);
			m_filerender->m_data_size -= buffsize;

			if (0 == m_filerender->m_data_size) {
				m_is_finish = true;
			}
		}
	}
	return 0;
}

FileRender *FileDataFilter::getFileRender(void)
{
	return this->m_filerender;
}

bool FileDataFilter::isFinish(void)
{
	return this->m_is_finish;
}

unsigned int FileDataFilter::getTargetId(void)
{
	if (this->m_filerender) {
		return this->m_filerender->m_target_id;
	}
	return 0;
}
