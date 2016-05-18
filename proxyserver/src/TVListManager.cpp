/*
 * TVListManager.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: hwj
 */

#include "TVListManager.h"
#include "SocketServer.h"
          
#include "sys_def.h"
#include "logger.h"

TVListManager *TVListManager::m_instance = new TVListManager();
TVListManager::TVListManager() {
	m_tvinfo_map.clear();
}

TVListManager::~TVListManager() {
	TVInfoMaps::iterator iter = this->m_tvinfo_map.begin();
		while (iter != this->m_tvinfo_map.end())
		{
			if (iter->second) {
				delete iter->second;
				iter->second = NULL;
			}
			this->m_tvinfo_map.erase(iter++);
		}
}

TVListManager *TVListManager::GetInstance()
{
	return m_instance;
}
int TVListManager::insertTVinfo(unsigned int id, TVInfoNode::DEVICES devices, bool isRefuse, int phpsockfd, int  nowtime)
{
	TVInfoNode *node = new TVInfoNode(id, devices, isRefuse, phpsockfd, nowtime);
	BaseManager::Locker lock(this);
	m_tvinfo_map[id] = node;
	LOG_DEBUG("m_tvinfo_map insert push id 0x%08x success. devices = %d", id, (int)devices);
	return 0;
}

bool TVListManager::isTVinfoExist(unsigned int id, TVInfoNode::DEVICES devices)
{
	BaseManager::Locker lock(this);
	TVInfoMaps::iterator iter = m_tvinfo_map.find(id);
	if(m_tvinfo_map.end() != iter)
	{
		if (iter->second) {
			if (devices == iter->second->getDevices()) {
				return true;
			}
		} else {
			m_tvinfo_map.erase(iter);
		}
	}
	return false;
}

int TVListManager::getPhpSock(unsigned int id)
{
	BaseManager::Locker lock(this);
	TVInfoMaps::iterator iter = m_tvinfo_map.find(id);
	if(m_tvinfo_map.end() != iter)
	{
		if(iter->second)
			return iter->second->getphpSockfd();
	}
	return -1;
}


bool TVListManager::getIsRefuse(unsigned int id)
{
    BaseManager::Locker lock(this);
	TVInfoMaps::iterator iter = m_tvinfo_map.find(id);
	if(m_tvinfo_map.end() != iter)
	{
		if(iter->second)
			return iter->second->getisResue();
	}
	return false;
}

int TVListManager::removeTvinfo(unsigned int id)
 {
	BaseManager::Locker lock(this);
	TVInfoMaps::iterator iter = m_tvinfo_map.find(id);
	if (this->m_tvinfo_map.end() != iter)
	{
		if (iter->second) 
		{
			delete iter->second;
			iter->second = NULL;
			this->m_tvinfo_map.erase(iter++);
			LOG_DEBUG("m_tvinfo_map remove push  id  0x%08x success.",id);
			return 1;
	   }
		else
		{
			return -1;
		}
	}
	return 0;
}


TVListManager::TVInfoNode *TVListManager::getNextNode(TVListManager::TVInfoMaps::iterator & iter)
{
      BaseManager::Locker lock(this);
     if (this->m_tvinfo_map.end() != iter)
     {
     	iter++;
     }
     else
     {
        return  NULL;
     }
   return  iter->second;
}

bool TVListManager::getTVListHead(TVListManager::TVInfoMaps::iterator &iter)
{
   BaseManager::Locker lock(this);
   if (this->m_tvinfo_map.empty()) {
	   return true;
   }

   iter = this->m_tvinfo_map.begin();
   return false;

}

TVListManager::TVInfoMaps & TVListManager::getTVInfoMaps(void)
{
	BaseManager::Locker lock(this);
	return m_tvinfo_map;
}
