/*
 * ConnectionManager.h
 *
 *  Created on: Dec 22, 2015
 *      Author: hwj
 */

#ifndef SRC_CONNECTIONMANAGER_H_
#define SRC_CONNECTIONMANAGER_H_

#include <map>
#include <pthread.h>
#include "BaseManager.h"

class Connection;

class ConnectionManager : public BaseManager {
public:

	static ConnectionManager *GetInstance();
	virtual ~ConnectionManager();

	Connection *getConnectionById(unsigned int id);
	Connection *getConnByReadSockfd(int sockfd);

	int insertConnection(unsigned int id, Connection *conn);
	int removeConnection(unsigned int id);

	int removeConnByReadSockfd(int sockfd);
	int getWriteSockfd(unsigned int id);
private:
	ConnectionManager();
	static ConnectionManager *m_instance;
	typedef std::map<unsigned int , Connection *> ConnectionsMap;
	ConnectionsMap m_conn_map;

};

#endif /* SRC_CONNECTIONMANAGER_H_ */
