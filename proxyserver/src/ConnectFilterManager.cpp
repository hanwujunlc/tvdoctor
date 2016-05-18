/*
 * ConnectFilterManager.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: hwj
 */

#include "ConnectFilterManager.h"
#include "ConnectFilter.h"
#include "sys_def.h"
#include "logger.h"

ConnectFilterManager *ConnectFilterManager::m_instance = new ConnectFilterManager();

ConnectFilterManager::ConnectFilterManager() {
	m_filter_map.clear();
}

ConnectFilterManager::~ConnectFilterManager() {
	ConnectFilterMaps::iterator iter = this->m_filter_map.begin();
	while (iter != this->m_filter_map.end()) {
		if (iter->second) {
			delete iter->second;
			iter->second = NULL;
		}
		this->m_filter_map.erase(iter++);
	}
}

ConnectFilterManager *ConnectFilterManager::GetInstance() {
	return m_instance;
}

int ConnectFilterManager::insertConnFilterMap(int sockfd, ConnectFilter *filter)
{
	if (!filter) return -1;
	BaseManager::Locker lock(this);
	this->m_filter_map[sockfd] = filter;
	LOG_DEBUG("insert conn filter sockfd =  %d", sockfd);
	return 0;
}

int ConnectFilterManager::insertConnFilterMap(int sockfd)
{
	ConnectFilter *filter = new ConnectFilter(sockfd);
	BaseManager::Locker lock(this);
	this->m_filter_map[sockfd] = filter;
	LOG_DEBUG("insert conn filter sockfd =  %d", sockfd);
	return 0;
}

ConnectFilter * ConnectFilterManager::getConnFilter(int sockfd)
{
	BaseManager::Locker lock(this);
	ConnectFilterMaps::iterator iter = this->m_filter_map.find(sockfd);
	if (this->m_filter_map.end() != iter) {
		LOG_DEBUG("find conn filter by sockfd %d success", sockfd);
		return iter->second;
	} else {
		LOG_DEBUG("find conn filter by sockfd %d fail", sockfd);
	}
	return NULL;
}

int ConnectFilterManager::removeConnFilter(int sockfd)
{
	BaseManager::Locker lock(this);
	ConnectFilterMaps::iterator iter = this->m_filter_map.find(sockfd);
	if (this->m_filter_map.end() != iter) {
		if(iter->second) {
			delete iter->second;
			iter->second = NULL;
		}
		this->m_filter_map.erase(iter++);
		LOG_DEBUG("remove conn filter by sockfd %d success", sockfd);
	} else {
		LOG_DEBUG("remove conn filter by sockfd %d fail", sockfd);
	}
	return 0;
}

bool ConnectFilterManager::isConnFilterExist(int sockfd)
{
	BaseManager::Locker lock(this);
	ConnectFilterMaps::iterator iter = this->m_filter_map.find(sockfd);
	if (this->m_filter_map.end() != iter) {
		LOG_DEBUG("conn filter by sockfd %d exist", sockfd);
		return true;
	}
	LOG_DEBUG("conn filter by sockfd %d no exist", sockfd);
	return false;
}

