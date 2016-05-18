/*
 * LogWebSockServer.cpp
 *
 *  Created on: Jan 13, 2016
 *      Author: hwj
 */

#include "LogWebSockServer.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <sstream>
#include <iostream>
#include <string>
#include <map>

#include "MessageBuffer.h"
#include "ConnectInfo.h"
#include "base64/base64.h"
#include "sha1/sha1.h"
#include "sys_def.h"
#include "logger.h"

LogWebSockServer::LogWebSockServer(int port) :
		BaseServer(port) {

}

LogWebSockServer::~LogWebSockServer() {
}

int LogWebSockServer::acceptConnect(int sockfd) {

	return 0;
}

int LogWebSockServer::disconnect(int sockfd) {
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


int LogWebSockServer::receiverMsg(int sockfd, char *buf, int len) {
	if (0 != strstr(buf, "\r\n\r\n")) {
		do_handshake(sockfd, buf, len);
	} else {
		do_parseMsg(sockfd, buf, len);
	}
	return 0;
}

int LogWebSockServer::do_handshake(int sockfd, char *buf, int len) {
	std::map<std::string, std::string> fields;
	std::istringstream s(buf);

	std::string header;
	std::string::size_type end;

// get headers
	while (std::getline(s, header) && header != "\r") {
		if (header[header.size() - 1] != '\r') {
			continue; // ignore malformed header lines?
		} else {
			header.erase(header.end() - 1);
		}

		end = header.find(": ", 0);

		if (end != std::string::npos) {
			std::string key = header.substr(0, end);
			std::string val = header.substr(end + 2);

			fields[key] = val;
		}
	}

	std::string server_key = fields["Sec-WebSocket-Key"];
	//LOG_DEBUG("server_key = %s", server_key.c_str());
	server_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	SHA1 sha;
	unsigned int message_digest[5];

	sha.Reset();
	sha << server_key.c_str();

	sha.Result(message_digest);
// convert sha1 hash bytes to network byte order because this sha1
//  library works on ints rather than bytes
	for (int i = 0; i < 5; i++) {
		message_digest[i] = htonl(message_digest[i]);
	}

	server_key = base64_encode(
			reinterpret_cast<const unsigned char*>(message_digest), 20);

	char head[1024] = { 0 };
	sprintf(head,
			"HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept:%s\r\nUpgrade: websocket\r\n\r\n",
			server_key.c_str());

	writeMsg(sockfd, head, (int) strlen(head));
	//LOG_DEBUG("send to %d byte %d buf", sockfd, (int) strlen(head));
	return 0;
}

int LogWebSockServer::do_parseMsg(int sockfd, char *buf, int len) {
	if (len < 2)
		return -1;
	int fin = ((buf[0] >> 7) & 1);
	int opcode = buf[0] & 0xf;
	int rsv = buf[0] & 0x70;
	int isMask = (buf[1] >> 7) & 1;
	int payload_len = (buf[1] & 0x7f);

	if (1 != fin) {
		LOG_DEBUG("fin Error:fin = %d", fin);
	}

	if (0 != rsv /*|| opcode > 2*/) {
		return -1;
	}

	if (8 == opcode) {
		LOG_DEBUG("Websocket close by client!");
		//disconnectBySockfd(sockfd);
		return 0;
	}

	int heade_length = 2;
	if (126 == payload_len) {
		heade_length += 2;
		payload_len = ((buf[2] & 0xff) << 8) + (buf[3] & 0xff);
	} else if (127 == payload_len) {
		//TODO
		heade_length += 8;
	} else {

	}
	char mask[4];
	bzero(mask, 4);
	if (1 == isMask) {
		memcpy(mask, buf + heade_length, 4);
		heade_length += 4;
	}
	char payload_data[BUFF_SIZE];
	bzero(payload_data, BUFF_SIZE);
	memcpy(payload_data, buf + heade_length, payload_len);

	if (1 == isMask) {
		//int i;
		for (int i = 0; i < payload_len; ++i) {
			//LOG_DEBUG("unmask data[%d] = %c", i, payload_data[i] ^ mask[i % 4]);
			payload_data[i] = payload_data[i] ^ mask[i % 4];
		}
	}

	parseDataPackage(sockfd, payload_data, payload_len);

	return 0;
}

int LogWebSockServer::parseDataPackage(int sockfd, char *buf, int len) {

	if (0 != strstr(buf, "\r\n\r\n")) {
		char *c = strstr(buf, "\r\n\r\n");
		char id[20];
		bzero(id, 20);
		memcpy(id, buf, c - buf);
		if (4 != strlen(id)) {
			LOG_DEBUG("tv id length ERROR length = %d", (int )strlen(id));
			return -1;
		}
		unsigned tv_id = ((id[0] & 0xff) << 24) + ((id[1] & 0xff) << 16)
				+ ((id[2] & 0xff) << 8) + (id[3] & 0xff);
		LOG_DEBUG("tv id  = 0x%08x", tv_id);

		insertPCConn(tv_id, sockfd);

	} else {
		//TODO

	}
	return 0;
}
