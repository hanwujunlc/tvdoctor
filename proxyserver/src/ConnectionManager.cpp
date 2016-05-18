/*
 * ConnectionManager.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: hwj
 */

#include "ConnectionManager.h"
#include "Connection.h"
#include "sys_def.h"
#include "logger.h"
ConnectionManager *ConnectionManager::m_instance = new ConnectionManager();

ConnectionManager::ConnectionManager() {
	m_conn_map.clear();
	//pthread_mutex_init(&m_mutex_lock, 0);

}

ConnectionManager::~ConnectionManager() {

	ConnectionsMap::iterator iter = this->m_conn_map.begin();
	while (iter != this->m_conn_map.end()) {
		if (iter->second) {
			delete iter->second;
			iter->second = NULL;
		}
		this->m_conn_map.erase(iter++);
	}

	//pthread_mutex_destroy(&m_mutex_lock);
}

ConnectionManager *ConnectionManager::GetInstance() {
	return m_instance;
}

Connection *ConnectionManager::getConnectionById(unsigned int id) {
	BaseManager::Locker lock(this);
	ConnectionsMap::iterator iter = this->m_conn_map.find(id);
	if (this->m_conn_map.end() != iter) {
		LOG_DEBUG("find connect by id = 0x%08x.", id);
		return iter->second;
	} else {
		LOG_DEBUG("can not find connect by id = 0x%08x.", id);
		return NULL;
	}
}

Connection *ConnectionManager::getConnByReadSockfd(int sockfd) {

	BaseManager::Locker lock(this);
	ConnectionsMap::iterator iter = this->m_conn_map.begin();
	while (this->m_conn_map.end() != iter) {
		if (iter->second) {
			if (sockfd == iter->second->getReadSockfd()) {
				LOG_DEBUG("find connect by Read Sockfd %d success.", sockfd);
				return iter->second;
			}
		}
		++iter;
	}
	LOG_DEBUG("can not find connect by Read Sockfd %d success.", sockfd);
	return NULL;
}

int ConnectionManager::insertConnection(unsigned int id, Connection *conn) {
	BaseManager::Locker lock(this);
	if (!conn)
		return -1;
	//this->m_conn_map.insert(std::make_pair(id, conn));
	this->m_conn_map[id] = conn;
	LOG_DEBUG("insert connect id = 0x%08x success.", id);
	return 0;
}

int ConnectionManager::removeConnection(unsigned int id) {
	BaseManager::Locker lock(this);
	ConnectionsMap::iterator iter = this->m_conn_map.find(id);
	if (this->m_conn_map.end() != iter) {
		if (iter->second) {
			delete iter->second;
		}
		this->m_conn_map.erase(iter);
		LOG_DEBUG("remove connect by id = 0x%08x success.", id);
	} else {
		LOG_DEBUG("remove connect by id = 0x%08x fail.", id);
	}
	return 0;
}

int ConnectionManager::removeConnByReadSockfd(int sockfd) {
	BaseManager::Locker lock(this);
	ConnectionsMap::iterator iter = this->m_conn_map.begin();
	while (this->m_conn_map.end() != iter) {
		if (iter->second) {
			if (sockfd == iter->second->getReadSockfd()) {
				delete iter->second;
				this->m_conn_map.erase(iter);
				LOG_DEBUG("remove connect by Read Sockfd = %d success.", sockfd);
				break;
			}
		}
		++iter;
	}
	LOG_DEBUG("remove connect by Read Sockfd = %d fail.", sockfd);
	return 0;
}

int ConnectionManager::getWriteSockfd(unsigned int id) {
	BaseManager::Locker lock(this);
	ConnectionsMap::iterator iter = this->m_conn_map.find(id);
	if (this->m_conn_map.end() != iter) {
		if (iter->second) {
			LOG_DEBUG("find write Sockfd by id = 0x%08x uccess.", id);
			return iter->second->getWriteSockfd();
		}
	}
	LOG_DEBUG("find write Sockfd by id = 0x%08x fail.", id);
	return -1;
}
