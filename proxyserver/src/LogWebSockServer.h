/*
 * LogWebSockServer.h
 *
 *  Created on: Jan 13, 2016
 *      Author: hwj
 */

#ifndef SRC_LOGWEBSOCKSERVER_H_
#define SRC_LOGWEBSOCKSERVER_H_

#include "BaseServer.h"

class LogWebSockServer: public BaseServer {
public:
	LogWebSockServer(int port);
	virtual ~LogWebSockServer();

	virtual int acceptConnect(int sockfd);
	virtual int disconnect(int sockfd);

	virtual int receiverMsg(int sockfd, char *buf, int len);


private:
	int do_handshake(int sockfd, char *buf, int len);
	int do_parseMsg(int sockfd, char *buf, int len);

	int parseDataPackage(int sockfd, char *buf, int len);
};

#endif /* SRC_LOGWEBSOCKSERVER_H_ */
