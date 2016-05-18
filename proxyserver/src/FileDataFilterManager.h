/*
 * FileDataFilterManager.h
 *
 *  Created on: Jan 4, 2016
 *      Author: hwj
 */

#ifndef SRC_FILEDATAFILTERMANAGER_H_
#define SRC_FILEDATAFILTERMANAGER_H_

#include "BaseManager.h"

#include <map>

class FileDataFilter;

class FileDataFilterManager: public BaseManager {
public:
	static FileDataFilterManager *GetInstance();

	virtual ~FileDataFilterManager();

	void insertFileFilter(int sockfd, FileDataFilter *filter);
	FileDataFilter *getFileFilterBySockfd(int sockfd);
	int removeFileFilterBySockfd(int sockfd);
	bool isFileFilterExist(int sockfd);

private:
	FileDataFilterManager();
	static FileDataFilterManager *m_instance;
	typedef std::map<int, FileDataFilter *> FileFilterMaps;
	FileFilterMaps m_filter_map;
};

#endif /* SRC_FILEDATAFILTERMANAGER_H_ */
