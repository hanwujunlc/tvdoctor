/*
 * LogSocketServer.h
 *
 *  Created on: Jan 13, 2016
 *      Author: hwj
 */

#ifndef SRC_LOGSOCKETSERVER_H_
#define SRC_LOGSOCKETSERVER_H_

#include "BaseServer.h"

class LogSocketServer: public BaseServer {
public:
	LogSocketServer(int port);
	virtual ~LogSocketServer();

	virtual int acceptConnect(int sockfd);
	virtual int disconnect(int sockfd);

	virtual int receiverMsg(int sockfd, char *buf, int len);

private:

};

#endif /* SRC_LOGSOCKETSERVER_H_ */
