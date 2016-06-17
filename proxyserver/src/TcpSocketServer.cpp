/*
 * TcpSocketServer.cpp
 *
 *  Created on: Dec 21, 2015
 *      Author: hwj
 */

#include "TcpSocketServer.h"
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "VncDataPackage.h"
#include "Connection.h"
#include "ConnectionManager.h"
#include "PackDataReader.h"
#include "ConnectFilterManager.h"
#include "ConnectFilter.h"

#include "sys_def.h"
#include "logger.h"
#include "VncUtils.h"
#include "TVListManager.h"
#include "TVOnlineListManager.h"
#include "CommandId.h"

TcpSocketServer::TcpSocketServer(int port) :
		SocketServer(port), m_port(port)
{
	// TODO Auto-generated constructor stub

}

TcpSocketServer::~TcpSocketServer() {
	// TODO Auto-generated destructor stub
}

int TcpSocketServer::acceptConnect(int sockfd) {
	ConnectFilter *filter = new ConnectFilter(sockfd);
	ConnectFilterManager::GetInstance()->insertConnFilterMap(sockfd, filter);
	return 0;
}

int TcpSocketServer::disconnect(int sockfd) {

	disconnectBySockfd(sockfd);

	return 0;
}

int TcpSocketServer::receiverMsg(int sockfd, char *buf, int len) {
	LOG_DEBUG("TcpSocketServer from %d receive MSG length = %d", sockfd, len);
	parseDataPackage(sockfd, buf, len);

	return 0;
}

int TcpSocketServer::sendMsg(int sockfd, const char *buf, int len) {

	LOG_DEBUG("TcpSocketServer send to %d MSG length =  %d", sockfd, len);
	int ret = writeMsg(sockfd, buf, len);

	return ret;
}

int TcpSocketServer::parseDataPackage(int sockfd, char *buf, int len) {

	ConnectFilter *filter = ConnectFilterManager::GetInstance()->getConnFilter(sockfd);
	if (!filter){
		return -1;
	}
	PackDataReader * reader = filter->getPackDataReader();
	if (!reader) {
		return -1;
	}
	reader->addBuffer((unsigned char *)buf, len);
	//std::queue<VncDataPackage *> pk_queue;
	VncDataPackage *package;
	while (reader->getPackQueue(&package)) {
		//while (!pk_queue.empty())
		{
		//	VncDataPackage *package = pk_queue.front();
		//	pk_queue.pop();
			if (!package) {
				return -1;
			}
			//const VncPackContent *content = package->getPackContent();
			//std::string buffer = package->getProtocolBuffer();
			//LOG_DEBUG("receive protocol buffer = %d", buffer.length());
			unsigned int source = package->getSource();
			unsigned int target = package->getTarget();
			LOG_DEBUG("source = 0x%08x; target = 0x%08x", source, target);
			// if (!content) {
			// 	return -1;
			// }
			unsigned int command_id = package->getCommandId();
			// content->m_command;

			LOG_DEBUG("command_id = 0x%08x", command_id);
			if (SERVER_ID == target) {
				switch (command_id) {
				case T2S_CMD_REG_TV: {
					//unsigned int id = content->m_intParam;
					const std::string & id = package->getStringParam(); //content->m_bufParam;
					LOG_DEBUG("tv_id = %s", id.c_str());
					unsigned int crc32_id = crc32_hash((const unsigned char*) (id.c_str()), id.length());
					
					bool tvidIsexist = TVListManager::GetInstance()->isTVinfoExist(crc32_id, TVListManager::TVInfoNode::DEVICES_PC);
                    LOG_DEBUG("crc32_id =  0x%08x,tvidIsexist=%d", crc32_id, tvidIsexist);
					if (tvidIsexist)
					{//存在 通知phpTV接受请求了
						int phpsck = TVListManager::GetInstance()->getPhpSock(crc32_id);
						LOG_DEBUG("phpsck === %d", phpsck);
						if (0 != phpsck) {
							writeMsgToPhpSocket(phpsck, "ok", 2);
						}
						TVListManager::GetInstance()->removeTvinfo(crc32_id);
						//TVOnlineListManager::GetInstance()->insertTVinfo(crc32_id, TVOnlineListManager::TVInfoNode::DEVICES_PC);
					}
					else
					{
						time_t nowtime = time(0);
						TVListManager::GetInstance()->insertTVinfo(crc32_id, TVListManager::TVInfoNode::DEVICES_TV, false, -1, nowtime);
					}

					Connection *conn = new Connection(sockfd, source, Connection::DEVICES_TV);
					ConnectionManager::GetInstance()->insertConnection(source,
							conn);

					VncDataPackage package;
					package.setSourceAndTarget(SERVER_ID, source);
					package.setCommandId(T2S_CMD_TELLME_SRV_ID, 0);
					package.setIntegerParam(SERVER_ID);
					std::string returnbuf = package.getProtocolBuffer();
					this->sendMsg(sockfd, returnbuf.c_str(),
							returnbuf.length());
				}
					break;
				case T2S_CMD_REG_TV_TEST: {
					const std::string & id = package->getStringParam(); //content->m_bufParam;
					LOG_DEBUG("tv_id = %s", id.c_str());
					unsigned int crc32_id = crc32_hash((const unsigned char*) (id.c_str()), id.length());

//					bool tvidIsexist = TVListManager::GetInstance()->isTVinfoExist(crc32_id, TVListManager::TVInfoNode::DEVICES_PC);
//					LOG_DEBUG("crc32_id =  0x%08x,tvidIsexist=%d", crc32_id, tvidIsexist);
//					if (tvidIsexist) { //存在 通知phpTV接受请求了
//						int phpsck = TVListManager::GetInstance()->getPhpSock(
//								crc32_id);
//						LOG_DEBUG("phpsck === %d", phpsck);
//						if (0 != phpsck) {
//							writeMsgToPhpSocket(phpsck, "ok", 2);
//						}
//						TVListManager::GetInstance()->removeTvinfo(crc32_id);
//						//TVOnlineListManager::GetInstance()->insertTVinfo(crc32_id, TVOnlineListManager::TVInfoNode::DEVICES_PC);
//					} else {
//						time_t nowtime = time(0);
//						TVListManager::GetInstance()->insertTVinfo(crc32_id,
//								TVListManager::TVInfoNode::DEVICES_TV, false,
//								-1, nowtime);
//					}

					Connection *conn = new Connection(sockfd, source,
							Connection::DEVICES_TV);
					ConnectionManager::GetInstance()->insertConnection(source,
							conn);

					VncDataPackage package;
					package.setSourceAndTarget(SERVER_ID, source);
					package.setCommandId(T2S_CMD_TELLME_SRV_ID, 0);
					package.setIntegerParam(SERVER_ID);
					std::string returnbuf = package.getProtocolBuffer();
					this->sendMsg(sockfd, returnbuf.c_str(), returnbuf.length());
				}
					break;
				case T2S_CMD_REJECT_HELP:
				{
				 const std::string & id = package->getStringParam(); 
					LOG_DEBUG("m_bufParam = %s", id.c_str());
					unsigned int crc32_id = crc32_hash((const unsigned char*) (id.c_str()), id.length());
					LOG_DEBUG("crc32_id =  0x%08x", crc32_id);
					bool   tvidIsexist = TVListManager::GetInstance()->isTVinfoExist(crc32_id, TVListManager::TVInfoNode::DEVICES_PC);
					if (tvidIsexist)
					{
						int phpsck =TVListManager::GetInstance()->getPhpSock(crc32_id);
						LOG_DEBUG("phpsck = %d", phpsck);
						writeMsgToPhpSocket(phpsck, "refuse", 6);
						TVListManager::GetInstance()->removeTvinfo(crc32_id);
					}
					else
					{
						TVListManager::GetInstance()->insertTVinfo(crc32_id, TVListManager::TVInfoNode::DEVICES_TV, true);
					}
				}
				break;
				case T2S_CMD_ANOTHER_PC_CTRL:
				{
					LOG_DEBUG("TV connect replace by other client");
					//const std::string & id = package->getStringParam();
					//Connection *conn = ConnectionManager::GetInstance()->getConnectionById(source);
					//if (conn) {
						disconnectBySockfd(sockfd, SocketServer::DISCONNECT_REPLACE);
					//}

				}break;
				case T2S_CMD_TV_EXIT_CTRL:
				{
					disconnectBySockfd(sockfd);

				}break;

				default:
					break;
				}
			} else {
				Connection *pc_conn =
						ConnectionManager::GetInstance()->getConnectionById(
								target);
				if (!pc_conn) {
					//TODO send error msg to TV
					LOG_ERROR("getConnectionByfd ERROR 2 ");
				} else {
					std::string returnbuf = package->getProtocolBuffer();
					this->writeMsgToWebsocket(pc_conn->getReadSockfd(), returnbuf.c_str(), returnbuf.length());
				}
			}
			delete package;
			package = NULL;
		}
	}
	return 0;
}

int TcpSocketServer::packageDataPackage(int sockfd, char *buf, int len) {
	return 0;
}

int TcpSocketServer::dealTimeOut() 
{
  return 0;
}
