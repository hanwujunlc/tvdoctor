/*
 * BaseServer.h
 *
 *  Created on: Jan 13, 2016
 *      Author: hwj
 */

#ifndef SRC_BASESERVER_H_
#define SRC_BASESERVER_H_

#include "BaseManager.h"
#include <map>

class ConnectInfo;
class MessageBuffer;

class BaseServer : public BaseManager{
public:
	BaseServer(int port);
	virtual ~BaseServer();

	void process();

	void processEpollEvents();

	int processMsg();

	virtual int acceptConnect(int sockfd) = 0;
	virtual int disconnect(int sockfd) = 0;
	virtual int receiverMsg(int sockfd, char *buf, int len) = 0;
	int writeMsg(int sockfd, const char *buf, int len);
	int writeMsgToWebSocket(int sockfd, const char *buf, int len);

protected:

	int insertPCConn(unsigned int id, int sockfd);
	int getPCConnByTVId(unsigned int id);
	int removePCConnBySock(int sockfd);

	typedef std::map<unsigned int, int> PCConnMaps; // key : tv id, value : websocket sockfd
	static PCConnMaps m_pcconn_map;

	int insertTVConn(int sockfd, ConnectInfo *info);
	ConnectInfo * getConnInfoByTVSock(int sockfd);
	int getPCSockByTVSock(int sockfd);
	int removeTVConnBySock(int sockfd);

	typedef std::map<int, ConnectInfo *> TvConnMaps; // key : tc tcp sockfd, value : ConnectInfo
	static TvConnMaps m_tvconn_map;

	int insertMessageBuffer(int sockfd, unsigned char *buf, unsigned int len);
	MessageBuffer *getMessageBuffer(int sockfd);
	bool isMessageBufferExist(int sockfd);

	typedef std::map<int, MessageBuffer *> MessageMaps;
	static MessageMaps m_message_map;

private:

	int init();
	void setnonblocking(int fd);
	int packageMsgForWebsocket(char *dest, char *src, int *len);

	int disconnectByTVSockfd(int sockfd);
	int m_socket;
	int m_port;
	int m_epollfd;

};

#endif /* SRC_BASESERVER_H_ */
