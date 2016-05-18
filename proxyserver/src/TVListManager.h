/*
 * TVListManager.h
 *
 *  Created on: Dec 16, 2015
 *      Author: hwj
 */

#ifndef SRC_TVLISTMANAGER_H_
#define SRC_TVLISTMANAGER_H_

#include <map>
#include <string>

#include "BaseManager.h"

class TVListManager: public BaseManager {
public:
	class TVInfoNode {
	public:
		enum DEVICES{
			DEVICES_TV = 0,
			DEVICES_PC,
		};
		TVInfoNode(unsigned int id, DEVICES devices, bool isRefuse = false, int phpsockfd = -1,
				int nowtime = 0) :
				m_id(id), m_phpsockfd(phpsockfd),  m_time(
						nowtime), m_devices(devices),m_isRefuse(isRefuse) {

		}
		int getphpSockfd() {
			return this->m_phpsockfd;
		}
		;
		unsigned int getTVid() {
			return this->m_id;
		}
		;
		int getTime() {
			return this->m_time;
		}

		int getDevices() {
			return this->m_devices;
		}

		bool getisResue() {
			return this->m_isRefuse;
		}
		;
	private:
		unsigned int m_id;
		int m_phpsockfd;
		int m_time;
		DEVICES m_devices;
		bool m_isRefuse;
	};

	static TVListManager *GetInstance();

	int insertTVinfo(unsigned int id, TVInfoNode::DEVICES devices, bool isRefuse = false, int phpsockfd = -1,
			int nowtime = 0);
	bool isTVinfoExist(unsigned int id, TVInfoNode::DEVICES devices);
	int getPhpSock(unsigned int id);
	bool getIsRefuse(unsigned int id);
	int removeTvinfo(unsigned int id);
	//int removeTimeoutinfo(int nowtime);
	//TVInfoNode getNextIter(TVInfoNode *);
	typedef std::map<unsigned int, TVInfoNode *> TVInfoMaps;
	bool getTVListHead(TVListManager::TVInfoMaps::iterator &iter);
	TVInfoNode *getNextNode(TVListManager::TVInfoMaps::iterator & iter);

	TVListManager::TVInfoMaps & getTVInfoMaps(void);

	virtual ~TVListManager();
private:
	TVListManager();

	TVInfoMaps m_tvinfo_map;

	static TVListManager *m_instance;

};

#endif /* SRC_TVLISTMANAGER_H_ */
