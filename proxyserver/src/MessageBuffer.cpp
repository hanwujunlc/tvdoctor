/*
 * MessageBuffer.cpp
 *
 *  Created on: Dec 23, 2015
 *      Author: hwj
 */

#include "MessageBuffer.h"
#include <string.h>

MessageBuffer::DataBuffer::DataBuffer(unsigned char *buf, unsigned int len) {
	m_buf = new unsigned char[len + 1];
	m_alloc_size = len + 1;
	bzero(m_buf, len + 1);
	memcpy(m_buf, buf, len);
	m_cache_size = len;
	m_pos_size = 0;

}

MessageBuffer::DataBuffer::~DataBuffer() {
	if (m_buf) {
		delete[] m_buf;
		m_buf = NULL;
	}
}

int MessageBuffer::DataBuffer::resetDataBuffer(unsigned int len) {


	m_cache_size -= len;
	unsigned char *buffer = new unsigned char[m_cache_size + 1];
	bzero(buffer, m_cache_size + 1);
	memcpy(buffer, m_buf + len, m_cache_size);
	delete[] m_buf;
	m_buf = NULL;

	m_buf = buffer;
	return 0;
}

MessageBuffer::MessageBuffer(int sockfd) {
	// TODO Auto-generated constructor stub
	while (!m_data_que.empty()) {
		m_data_que.pop();
	}
}

MessageBuffer::~MessageBuffer() {
	// TODO Auto-generated destructor stub
	while (!m_data_que.empty()) {
		DataBuffer *buffer = m_data_que.front();
		if (buffer) {
			delete buffer;
			buffer = NULL;
		}
		m_data_que.pop();
	}
}

void MessageBuffer::insertDataBuffer(unsigned char *buf, unsigned int len) {
	DataBuffer *buffer = new DataBuffer(buf, len);
	m_data_que.push(buffer);
}

void MessageBuffer::insertDataBuffer(DataBuffer *buffer) {
	m_data_que.push(buffer);
}

MessageBuffer::DataBuffer *MessageBuffer::getDataBuffer(void) {
	if (m_data_que.empty()) {
		return NULL;
	}

	return m_data_que.front();
}

int MessageBuffer::removeDataBufferFont(void)
{
	DataBuffer *buffer = m_data_que.front();
	if (buffer) {
		delete buffer;
		buffer = NULL;
	}
	m_data_que.pop();
	return 0;
}

bool MessageBuffer::isDataBufferEmpty(void) {
	return m_data_que.empty();
}

int MessageBuffer::getDataBufferSize(void)
{
	return m_data_que.size();
}
