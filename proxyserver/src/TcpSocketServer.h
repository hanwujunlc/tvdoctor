/*
 * TcpSocketServer.h
 *
 *  Created on: Dec 21, 2015
 *      Author: hwj
 */

#ifndef SRC_TCPSOCKETSERVER_H_
#define SRC_TCPSOCKETSERVER_H_

#include "SocketServer.h"

class TcpSocketServer: public SocketServer {
public:
	TcpSocketServer(int port);
	virtual ~TcpSocketServer();

	virtual int acceptConnect(int sockfd);
	virtual int disconnect(int sockfd);

	virtual int receiverMsg(int sockfd, char *buf, int len);

	virtual int sendMsg(int sockfd, const char *buf, int len);

	virtual int parseDataPackage(int sockfd, char *buf, int len);
	virtual int packageDataPackage(int sockfd, char *buf, int len);
    virtual int dealTimeOut() ;
private:
	int transferMsg();
	int processMsg();
	int m_port;
};

#endif /* SRC_TCPSOCKETSERVER_H_ */
