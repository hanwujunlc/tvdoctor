/*
 * FileSocketServer.h
 *
 *  Created on: Dec 31, 2015
 *      Author: hwj
 */

#ifndef SRC_FILESOCKETSERVER_H_
#define SRC_FILESOCKETSERVER_H_

#include "SocketServer.h"

class FileSocketServer: public SocketServer {
public:
	FileSocketServer(int port);
	virtual ~FileSocketServer();

	virtual int acceptConnect(int sockfd);
	virtual int disconnect(int sockfd);

	virtual int receiverMsg(int sockfd, char *buf, int len);
	virtual int sendMsg(int sockfd, const char *buf, int len);

	virtual int parseDataPackage(int sockfd, char *buf, int len);
	virtual int packageDataPackage(int sockfd, char *buf, int len);
	virtual int dealTimeOut() ;
private:

};

#endif /* SRC_FILESOCKETSERVER_H_ */
