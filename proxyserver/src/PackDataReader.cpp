/*
 * PackDataReader.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: hwj
 */

#include "PackDataReader.h"
#include "VncDataPackage.h"
#include "sys_def.h"
#include "logger.h"

#include <string.h>

PackDataReader::PackDataReader() :
	m_cached_size(0),
	m_pkage_size(0),
	m_getpkage(false)
{
	while(!m_data.empty())
		m_data.pop();

	m_cache_buf = new unsigned char[BUFF_SIZE];
	m_alloc_size = BUFF_SIZE;

}

PackDataReader::~PackDataReader() {
	if (m_cache_buf) {
		delete [] m_cache_buf;
		m_cache_buf = NULL;
	}
	while(!m_data.empty()) {
		VncDataPackage *package = m_data.front();
		if (package) {
			delete package;
			package = NULL;
		}
		m_data.pop();
	}
}

int PackDataReader::addBuffer(unsigned char *buf, unsigned int buffsize)
{
	if(!buf || 0 == buffsize) {
		return -1;
	}
	LOG_DEBUG("addBuffer buffsize = %u, m_pkage_size = %u, m_cached_size = %u", buffsize, m_pkage_size, m_cached_size);

	if(m_cached_size + buffsize > m_alloc_size) {
		m_alloc_size += BUFF_SIZE;
		unsigned char * newbuff = new unsigned char[m_alloc_size];
		memcpy(newbuff, m_cache_buf, m_cached_size);
		delete[] m_cache_buf;
		m_cache_buf = NULL;
		m_cache_buf = newbuff;
		LOG_DEBUG("addBuffer new alloc buff size %u", m_alloc_size);
	}

	memcpy(&m_cache_buf[m_cached_size], buf, buffsize);
	m_cached_size += buffsize;

	while(m_cached_size >= 4 && m_cached_size >= m_pkage_size) {
		if(0 == m_pkage_size) {
			m_pkage_size = (m_cache_buf[0] << 24) | (m_cache_buf[1] << 16) | (m_cache_buf[2] << 8) | (m_cache_buf[3]);
			LOG_DEBUG("addBuffer m_pkage_size = %u", m_pkage_size);
		}
		if (0 == m_pkage_size) {
			m_cached_size = 0;
			bzero(m_cache_buf, m_alloc_size);
			break;
		}

		if(m_cached_size >= m_pkage_size) 
		{
			if(m_pkage_size > 8) {
				//LOG_DEBUG("addBuffer new package 1 ~ m_pkage_size = %u!", m_pkage_size);
				VncDataPackage * package = new VncDataPackage(m_cache_buf, m_pkage_size);
				m_data.push(package);

				//LOG_DEBUG("addBuffer new package~ !m_pkage_size = %u!", m_pkage_size);
			}
			//LOG_DEBUG("addBuffer 2 m_pkage_size = %u, m_cached_size = %u", m_pkage_size, m_cached_size);

			char swap_buf[m_cached_size - m_pkage_size + 1];
			bzero(swap_buf, m_cached_size - m_pkage_size + 1);
			memcpy(swap_buf, m_cache_buf + m_pkage_size, m_cached_size - m_pkage_size);
			bzero(m_cache_buf, m_alloc_size);
			memcpy(m_cache_buf, swap_buf, m_cached_size - m_pkage_size);
			//LOG_DEBUG("addBuffer 3 m_pkage_size = %u, m_cached_size = %u", m_pkage_size, m_cached_size);
			m_cached_size -= m_pkage_size;
			m_pkage_size = 0;
			LOG_DEBUG("addBuffer m_pkage_size = %u, m_cached_size = %u", m_pkage_size, m_cached_size);
		}
	}
	return 0;
}

bool PackDataReader::getPackQueue(std::queue<VncDataPackage*> & queue)
{
	if (this->m_data.empty()) {
		return false;
	}
	queue = this->m_data;
	return true;
}
bool PackDataReader::getPackQueue(VncDataPackage ** package)
{
	if (this->m_data.empty()) {
		return false;
	}
	VncDataPackage * pk = this->m_data.front();
	this->m_data.pop();
	*package = pk;
	//delete pk;
	return true;
}
