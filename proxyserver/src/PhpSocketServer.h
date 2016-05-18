/*
 * PhpSocketServer.h
 *
 *  Created on: Dec 30, 2015
 *      Author: hwj
 */

#ifndef SRC_PHPSOCKETSERVER_H_
#define SRC_PHPSOCKETSERVER_H_

#include "SocketServer.h"

class PhpSocketServer: public SocketServer {
public:
	PhpSocketServer(int port);
	virtual ~PhpSocketServer();

	virtual int acceptConnect(int sockfd);
	virtual int disconnect(int sockfd);
	virtual int receiverMsg(int sockfd, char *buf, int len);

	virtual int sendMsg(int sockfd, const char *buf, int len);

	virtual int parseDataPackage(int sockfd, char *buf, int len);
	virtual int packageDataPackage(int sockfd, char *buf, int len);
    virtual int dealTimeOut();
private:
  //  void dealPhpSocketTimeout();
private:
	int m_port;
    int m_count;
    bool m_isFistStartTimer;
};

#endif /* SRC_PHPSOCKETSERVER_H_ */
