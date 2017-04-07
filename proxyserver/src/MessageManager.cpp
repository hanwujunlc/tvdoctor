/*
 * MessageManager.cpp
 *
 *  Created on: Dec 17, 2015
 *      Author: hwj
 */

#include "MessageManager.h"
#include "MessageBuffer.h"
#include "sys_def.h"
#include "logger.h"

MessageManager *MessageManager::m_instance = new MessageManager();

MessageManager::MessageManager() {
	m_message_map.clear();
}

MessageManager::~MessageManager() {
	MessageMaps::iterator iter = m_message_map.begin();
	while (iter != m_message_map.end()) {
		if (iter->second) {
			delete iter->second;
			iter->second = NULL;
		}
		m_message_map.erase(iter++);
	}
}

MessageManager *MessageManager::GetInstance()
{
	return m_instance;
}

int MessageManager::insertMessageBuffer(int sockfd, unsigned char *buf, unsigned int len)
{
	MessageBuffer *buffer;
	BaseManager::Locker lock(this);

	MessageMaps::iterator iter = m_message_map.find(sockfd);
	if (m_message_map.end() == iter) {
		buffer = new MessageBuffer(sockfd);
		LOG_DEBUG("new  Message Buffer by socket = %d", sockfd);
	} else {
		buffer = iter->second;
	}
	if (buffer) {
		buffer->insertDataBuffer(buf, len);
	}
	LOG_DEBUG("insert Message Buffer by socket = %d", sockfd);
	return 0;
}

int MessageManager::removeMessageBuffer(int sockfd)
{
	BaseManager::Locker lock(this);
	MessageMaps::iterator iter = m_message_map.find(sockfd);
	if (m_message_map.end() != iter) {
		if (iter->second) {
			delete iter->second;
			iter->second = NULL;
		}
		m_message_map.erase(iter);
		LOG_DEBUG("Remove Message Buffer by socket = %d success", sockfd);
	} else {
		LOG_DEBUG("Remove Message Buffer by socket = %d fail", sockfd);
	}

	return 0;
}

MessageBuffer *MessageManager::getMessageBuffer(int sockfd)
{
	BaseManager::Locker lock(this);
	MessageMaps::iterator iter = m_message_map.find(sockfd);
	if (m_message_map.end() != iter) {
		return iter->second;
	}
	return NULL;
}

bool MessageManager::isMessageBufferExist(int sockfd)
{
	BaseManager::Locker lock(this);
	MessageMaps::iterator iter = m_message_map.find(sockfd);
	if (m_message_map.end() != iter) {
		return true;
	}
	return false;
}

MessageManager::MessageMaps & MessageManager::getMessageMap(void)
{
	return m_message_map;
}
