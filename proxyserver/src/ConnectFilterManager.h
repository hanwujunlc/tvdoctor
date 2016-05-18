/*
 * ConnectFilterManager.h
 *
 *  Created on: Jan 4, 2016
 *      Author: hwj
 */

#ifndef SRC_CONNECTFILTERMANAGER_H_
#define SRC_CONNECTFILTERMANAGER_H_

#include "BaseManager.h"

#include <map>

class ConnectFilter;

class ConnectFilterManager: public BaseManager {
public:
	static ConnectFilterManager *GetInstance();

	virtual ~ConnectFilterManager();

	int insertConnFilterMap(int sockfd, ConnectFilter *filter);
	int insertConnFilterMap(int sockfd);
	ConnectFilter * getConnFilter(int sockfd);
	int removeConnFilter(int sockfd);
	bool isConnFilterExist(int sockfd);
private:
	static ConnectFilterManager *m_instance;

	ConnectFilterManager();

	typedef std::map<int, ConnectFilter *> ConnectFilterMaps;
	ConnectFilterMaps m_filter_map;

};

#endif /* SRC_CONNECTFILTERMANAGER_H_ */
