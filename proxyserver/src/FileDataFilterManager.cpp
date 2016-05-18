/*
 * FileDataFilterManager.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: hwj
 */

#include "FileDataFilterManager.h"
#include "FileDataFilter.h"
#include "sys_def.h"
#include "logger.h"

FileDataFilterManager *FileDataFilterManager::m_instance = new FileDataFilterManager();

FileDataFilterManager::FileDataFilterManager() {
	m_filter_map.clear();
}

FileDataFilterManager::~FileDataFilterManager() {

	FileFilterMaps::iterator iter = this->m_filter_map.begin();
	while (iter != this->m_filter_map.end()) {
		if (iter->second) {
			delete iter->second;
			iter->second = NULL;
		}
		this->m_filter_map.erase(iter++);
	}

}

FileDataFilterManager *FileDataFilterManager::GetInstance() {
	return m_instance;
}

void FileDataFilterManager::insertFileFilter(int sockfd, FileDataFilter *filter)
{
	if (!filter) return;
	BaseManager::Locker lock(this);
	m_filter_map[sockfd] = filter;
	LOG_DEBUG("insert FileFilter By Sockfd = %d !", sockfd);
}

FileDataFilter *FileDataFilterManager::getFileFilterBySockfd(int sockfd)
{
	BaseManager::Locker lock(this);
	FileFilterMaps::iterator iter = m_filter_map.find(sockfd);
	if (m_filter_map.end() != iter) {
		LOG_DEBUG("get FileFilter By Sockfd = %d success!", sockfd);
		return iter->second;
	}
	LOG_DEBUG("get FileFilter By Sockfd = %d success!", sockfd);
	return NULL;
}

int FileDataFilterManager::removeFileFilterBySockfd(int sockfd)
{
	BaseManager::Locker lock(this);
	FileFilterMaps::iterator iter = m_filter_map.find(sockfd);
	if (m_filter_map.end() != iter) {
		if (iter->second) {
			delete iter->second;
			iter->second = NULL;
			LOG_DEBUG("remove FileFilter By Sockfd = %d success!", sockfd);
		}
		this->m_filter_map.erase(iter++);
		return 0;
	}
	LOG_DEBUG("remove FileFilter By Sockfd = %d fail!", sockfd);
	return -1;
}

bool FileDataFilterManager::isFileFilterExist(int sockfd)
{
	BaseManager::Locker lock(this);
	FileFilterMaps::iterator iter = m_filter_map.find(sockfd);
	if (m_filter_map.end() != iter) {
		return true;
	}
	return false;
}


