/*
 * LogSocketServer.cpp
 *
 *  Created on: Jan 13, 2016
 *      Author: hwj
 */

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "LogSocketServer.h"
#include "ConnectInfo.h"
#include "sys_def.h"
#include "logger.h"

LogSocketServer::LogSocketServer(int port) :
	BaseServer(port)
{

}

LogSocketServer::~LogSocketServer() {
}

int LogSocketServer::acceptConnect(int sockfd){

	return 0;
}

int LogSocketServer::disconnect(int sockfd){

//	removeConnFilter(sockfd);
//	Connection *conn = getConnByReadSockfd(sockfd);
//	if (conn) {
//		unsigned int id = conn->getReadfd();
//		unsigned int wid = conn->getWriteId();
//		int wsockfd = conn->getWritefd();
//		//TODO
//		//disconnectToTcpSocket(wsockfd, wid);
//		removeConnection(id);
//		removeConnection(wid);
//		removeConnFilter(wsockfd);
//	}
	return 0;
}


int LogSocketServer::receiverMsg(int sockfd, char *buf, int len){

	if (0 != strstr(buf, "\r\n\r\n")) {
		char *c = strstr(buf, "\r\n\r\n");
		char id[20];
		bzero(id, 20);
		memcpy(id, buf, c - buf);
		if(4 != strlen(id)) {
			LOG_ERROR("tv id length ERROR length = %d", (int)strlen(id));
			return -1;
		}
		unsigned tv_id = ((id[0] & 0xff) << 24) + ((id[1] & 0xff) << 16) + ((id[2] & 0xff) << 8) + (id[3] & 0xff);
		LOG_DEBUG("tv id  = 0x%08x", tv_id);

		int pc_sockfd = getPCConnByTVId(tv_id);
		if (-1 != pc_sockfd) {
			ConnectInfo *info = new ConnectInfo(tv_id, pc_sockfd, sockfd);
			insertTVConn(sockfd, info);
		}

	} else {
		ConnectInfo *info = this->getConnInfoByTVSock(sockfd);

		if (info) {
			writeMsgToWebSocket(info->getPCSockfd(), buf, len);
		} else {
			LOG_ERROR("tv info can not find by sockfd = %d", sockfd);
			//TODO disconnect the connect client
		}
	}

	return 0;
}
