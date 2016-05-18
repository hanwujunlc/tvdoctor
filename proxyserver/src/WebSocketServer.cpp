/*
 * WebSocketServer.cpp
 *
 *  Created on: Dec 21, 2015
 *      Author: hwj
 */

#include "WebSocketServer.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <sstream>
#include <iostream>
#include <string>
#include <map>

#include "VncUtils.h"
#include "VncDataPackage.h"
#include "Connection.h"
#include "ConnectionManager.h"
#include "ConnectFilter.h"
#include "ConnectFilterManager.h"
#include "TVOnlineListManager.h"
#include "base64/base64.h"
#include "PackDataReader.h"
#include "sha1/sha1.h"
#include "sys_def.h"
#include "logger.h"
#include "CommandId.h"

WebSocketServer::WebSocketServer(int port) :
		SocketServer(port), m_port(port) {

}

WebSocketServer::~WebSocketServer() {
	// TODO Auto-generated destructor stub
}

int WebSocketServer::acceptConnect(int sockfd) {
	ConnectFilter *filter = new ConnectFilter(sockfd);
	ConnectFilterManager::GetInstance()->insertConnFilterMap(sockfd, filter);
	return 0;
}

int WebSocketServer::disconnect(int sockfd) {

	disconnectBySockfd(sockfd);
//	ConnectFilterManager::GetInstance()->removeConnFilter(sockfd);
//
//	Connection *conn = ConnectionManager::GetInstance()->getConnByReadSockfd(sockfd);
//	if (conn) {
//		unsigned int id = conn->getReadfd();
//		unsigned int wid = conn->getWriteId();
//		int wsockfd = conn->getWritefd();
//		//this->disconnectToWebSocket(sockfd, id);
//		this->disconnectToTcpSocket(wsockfd, wid);
//
//		ConnectionManager::GetInstance()->removeConnection(id);
//		ConnectionManager::GetInstance()->removeConnection(wid);
//
//		ConnectFilterManager::GetInstance()->removeConnFilter(wsockfd);
//	}
	return 0;
}

int WebSocketServer::receiverMsg(int sockfd, char *buf, int len) {
	if (0 != strstr(buf, "\r\n\r\n")) {
		do_handshake(sockfd, buf, len);
	} else {
		do_parseMsg(sockfd, buf, len);
	}
	return 0;
}

int WebSocketServer::sendMsg(int sockfd, const char *buf, int len) {

	char web_buf[BUFF_SIZE];
	bzero(web_buf, BUFF_SIZE);

	packageMsgForWebsocket(web_buf, (char *)buf, &len);
	LOG_DEBUG("WebSocketServer send to %d MSG length =  %d", sockfd, len);
	int ret = writeMsg(sockfd, web_buf, len);
	return ret;
}

int WebSocketServer::parseDataPackage(int sockfd, char *buf, int len) {

	ConnectFilter *filter = ConnectFilterManager::GetInstance()->getConnFilter(sockfd);
	if (!filter) {
		return -1;
	}
	PackDataReader * reader = filter->getPackDataReader();
	if (!reader) {
		return -1;
	}
	reader->addBuffer((unsigned char *)buf, len);
	VncDataPackage *package;
	while (reader->getPackQueue(&package)) {
		//while (!pk_queue.empty()) 
		{
			//VncDataPackage *package = pk_queue.front();
			if (!package) {
				return -1;
			}
			//const VncPackContent *content = package->getPackContent();
			//std::string buffer = package->getProtocolBuffer();
			//LOG_DEBUG("receive protocol buffer = %d", (int )buffer.length());
			unsigned int source =  package->getSource();
			unsigned int target = package->getTarget();
			LOG_DEBUG("source = 0x%08x; target = 0x%08x", source, target);
			// if (!content) {
			// 	return -1;
			// }
			unsigned int command_id = package->getCommandId();//content->m_command;
			LOG_DEBUG("command_id =  0x%08x", command_id);

			if (SERVER_ID == target) {
				switch (command_id) {
				case CMD_REG_PC:
				{// PC register on Server

					Connection *conn = new Connection(sockfd, source, Connection::DEVICES_PC);

					//unsigned int id = content->m_intParam;
					const std::string & id = package->getStringParam();//content->m_bufParam;

					LOG_DEBUG("source = 0x%08x TV ID = %s", source, id.c_str());
					unsigned int crc32_id = crc32_hash(
							(const unsigned char*) (id.c_str()), id.length());
					LOG_DEBUG("crc32_id =  0x%08x", crc32_id);
					Connection *tv_conn =
							ConnectionManager::GetInstance()->getConnectionById(
									crc32_id);
					if (!tv_conn) {
						//TODO send error msg to PC
						LOG_DEBUG("getConnectionByfd ERROR");
					} else {
						tv_conn->setWriteSockfd(sockfd);
						tv_conn->setWriteId(source);
						conn->setWriteSockfd(tv_conn->getReadSockfd());
						conn->setWriteId(crc32_id);

						ConnectionManager::GetInstance()->insertConnection(
								source, conn);

						VncDataPackage package1;
						package1.setSourceAndTarget(SERVER_ID, source);
						package1.setCommandId(CMD_TELLME_SRV_ID, 0);
						package1.setIntegerParam(SERVER_ID);
						std::string returnbuf1 = package1.getProtocolBuffer();
						this->sendMsg(sockfd, returnbuf1.c_str(),
								returnbuf1.length());

						VncDataPackage package2;
						package2.setSourceAndTarget(SERVER_ID, source);
						package2.setCommandId(CMD_TELLME_TV_ID, 0);
						package2.setIntegerParam(crc32_id);
						std::string returnbuf2 = package2.getProtocolBuffer();
						this->sendMsg(sockfd, returnbuf2.c_str(),
								returnbuf2.length());

						VncDataPackage package3;
						package3.setSourceAndTarget(SERVER_ID, crc32_id);
						package3.setCommandId(T2S_CMD_TELLME_PC_ID, 0);
						package3.setIntegerParam(source);
						std::string returnbuf3 = package3.getProtocolBuffer();
						this->writeMsgToTcpsocket(tv_conn->getReadSockfd(),
								returnbuf3.c_str(), returnbuf3.length());

						TVOnlineListManager::GetInstance()->insertTVinfo(crc32_id,
													TVOnlineListManager::TVInfoNode::DEVICES_PC);
					}
				}
					break;
				case CMD_PC_NEED_EXIT:
				{ //pc disconnect to server
					Connection *pc_conn = ConnectionManager::GetInstance()->getConnectionById(
													source);
					if (pc_conn) {
						//unsigned int id = conn->getReadfd();
						//unsigned int wid = conn->getWriteId();


						int wsockfd = pc_conn->getWriteSockfd();

						this->disconnectToWebSocket(sockfd, source);
						this->disconnectToTcpSocket(wsockfd, target);

						ConnectionManager::GetInstance()->removeConnection(source);
						ConnectionManager::GetInstance()->removeConnection(target);

						ConnectFilterManager::GetInstance()->removeConnFilter(sockfd);
						ConnectFilterManager::GetInstance()->removeConnFilter(wsockfd);
					} else {

					}
				}break;
				default:
					break;
				}
			} else {
				Connection *tv_conn =
						ConnectionManager::GetInstance()->getConnectionById(
								target);
				if (!tv_conn) {
					LOG_DEBUG("getConnectionByfd ERROR 2 ");
				} else {
					//LOG_DEBUG("writeMsgToTcpsocket 1 ");
					std::string returnbuf = package->getProtocolBuffer();
					this->writeMsgToTcpsocket(tv_conn->getReadSockfd(), returnbuf.c_str(), returnbuf.length());
				}
			}
			delete package;
			package = NULL;
		}
	}
	return 0;
}

int WebSocketServer::packageDataPackage(int sockfd, char *buf, int len) {

	return 0;
}

int WebSocketServer::do_handshake(int sockfd, char *buf, int len) {
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

int WebSocketServer::do_parseMsg(int sockfd, char *buf, int len) {
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

	if (8 == opcode) {
		LOG_DEBUG("Websocket close by client!");
		this->removeSockInEpoll(sockfd);
		this->disconnectBySockfd(sockfd);
		return 0;
	}

	if (0 != rsv /*|| opcode > 2*/) {
		return -1;
	}
	int heade_length = 2;
	if (126 == payload_len) {
		heade_length += 2;
		payload_len = ((buf[2] & 0xff) << 8) + (buf[3] & 0xff);
		LOG_DEBUG("payload_len extern = %d", payload_len);
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
//	LOG_DEBUG("payload data = %s", payload_data);

	parseDataPackage(sockfd, payload_data, payload_len);

	return 0;
}

int WebSocketServer::do_packageMsg(std::string & dest, std::string & src) {
//TODO
	if (src.empty()) {
		return -1;
	}
	if (!dest.empty()) {
		dest.clear();
	}
	dest[0] = 0x81;
	dest[1] = src.size();
	dest += src;
	return 0;
}

int WebSocketServer::do_packageMsg(char *dest, char *src, int *len) {
	if (!dest || !src || !len) {
		return -1;
	}
	int length = *len;
	LOG_DEBUG("length = %d, %02X", length, length);
	dest[0] = 0x82;
	int len_pos = 2;
//	if (length > 65535) {
//		dest[1] = 127;
//		dest[2] = (length >> 56) & 0xFF;
//		dest[3] = (length >> 48) & 0xFF;
//		dest[4] = (length >> 40) & 0xFF;
//		dest[5] = (length >> 32) & 0xFF;
//		dest[6] = (length >> 24) & 0xFF;
//		dest[7] = (length >> 16) & 0xFF;
//		dest[8] = (length >> 8) & 0xFF;
//		dest[9] = (length) & 0xFF;
//
//		len_pos += 8;
//	} else
		if (length >= 126) {
		dest[1] = 126;
		dest[2] = (length >> 8) & 0xFF;
		dest[3] = (length) & 0xFF;
		len_pos += 2;
	} else {
		dest[1] = length & 0x7f;
	}

	LOG_DEBUG("dest[1] = %02X", dest[1] & 0xFF);
	memcpy(dest + len_pos, src, length);
	*len += len_pos;
	return 0;
}

int WebSocketServer::dealTimeOut() 
{
    return 0 ;
}
