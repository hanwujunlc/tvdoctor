/*
 * SocketServer.h
 *
 *  Created on: Dec 15, 2015
 *      Author: hwj
 */

#ifndef SRC_SOCKETSERVER_H_
#define SRC_SOCKETSERVER_H_

class PackDataReader;

class SocketServer{
public:
	enum DISCONNECT_ENUM {
		DISCONNECT_REPLACE = 0,
	};
	SocketServer(int port);
	virtual ~SocketServer();

	void processEvents();

	virtual int acceptConnect(int sockfd) = 0;
	virtual int disconnect(int sockfd) = 0;

	virtual int receiverMsg(int sockfd, char *buf, int len) = 0;
	virtual int sendMsg(int sockfd, const char *buf, int len) = 0;

	virtual int parseDataPackage(int sockfd, char *buf, int len) = 0;
	virtual int packageDataPackage(int sockfd, char *buf, int len) = 0;

	virtual int dealTimeOut() = 0;

	int writeMsgToWebsocket(int sockfd, const char *buf, int len);

	int writeMsgToTcpsocket(int sockfd, const char *buf, int len);

	int packageMsgForWebsocket(char *dest, char *src, int *len);

	int writeMsg(int sockfd, const char *buf, int len);
     
    int writeMsgToPhpSocket(int sockfd, const char *buf, int len);

    int removeSockInEpoll(int sockfd);
	int disconnectBySockfd(int sockfd);
	int disconnectBySockfd(int sockfd, DISCONNECT_ENUM how);

	int disconnectToWebSocket(int sockfd, unsigned int id);
	int disconnectToWebSocket(int sockfd, unsigned int id, DISCONNECT_ENUM how);

	int disconnectToTcpSocket(int sockfd, unsigned int id);

protected:
	//PackDataReader *m_pack_reader;
private:
	int init();
	void setnonblocking(int fd);
	int socket_set_keepalive (int fd);

	void handlEvent(int sockfd, char *buf, int len);

	int m_socket;
	int m_port;
	int m_epollfd;


};

#endif /* SRC_SocketServer_H_ */
