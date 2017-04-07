/*
 * WebSocketServer.h
 *
 *  Created on: Dec 21, 2015
 *      Author: hwj
 */

#ifndef SRC_WEBSOCKETSERVER_H_
#define SRC_WEBSOCKETSERVER_H_

#include "SocketServer.h"

#include <string>

class WebSocketServer: public SocketServer {
public:
	WebSocketServer(int port);
	virtual ~WebSocketServer();

	virtual int acceptConnect(int sockfd);
	virtual int disconnect(int sockfd);
	virtual int receiverMsg(int sockfd, char *buf, int len);

	virtual int sendMsg(int sockfd, const char *buf, int len);

	virtual int parseDataPackage(int sockfd, char *buf, int len);
	virtual int packageDataPackage(int sockfd, char *buf, int len);
    virtual int dealTimeOut() ;
private:
	int do_handshake(int sockfd, char *buf, int len);
	int do_parseMsg(int sockfd, char *buf, int len);
	int do_parsebuffer(int sockfd, char *buf, int len);

	int do_packageMsg(std::string & dest, std::string & src);

	int do_packageMsg(char *dest, char *src, int *len);

	int m_port;
};

#endif /* SRC_WEBSOCKETSERVER_H_ */
