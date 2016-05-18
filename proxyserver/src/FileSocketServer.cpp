/*
 * FileSocketServer.cpp
 *
 *  Created on: Dec 31, 2015
 *      Author: hwj
 */

#include "FileSocketServer.h"

#include "FileRender.h"
#include "FileDataFilterManager.h"
#include "FileDataFilter.h"
#include "ConnectionManager.h"
#include "Connection.h"
#include "VncDataPackage.h"

#include "sys_def.h"
#include "logger.h"
FileSocketServer::FileSocketServer(int port) :
		SocketServer(port) {


}

FileSocketServer::~FileSocketServer() {

}

int FileSocketServer::acceptConnect(int sockfd) {
	FileDataFilter * filter = new FileDataFilter(sockfd);
	FileDataFilterManager::GetInstance()->insertFileFilter(sockfd, filter);
	return 0;
}

int FileSocketServer::disconnect(int sockfd) {
	removeSockInEpoll(sockfd);
	FileDataFilterManager::GetInstance()->removeFileFilterBySockfd(sockfd);
	return 0;
}

int FileSocketServer::receiverMsg(int sockfd, char *buf, int len) {
	//LOG_DEBUG("FileSocketServer from %d receive MSG length = %d", sockfd, len);
	parseDataPackage(sockfd, buf, len);
	return 0;
}

int FileSocketServer::sendMsg(int sockfd, const char *buf, int len) {
	return 0;
}

int FileSocketServer::parseDataPackage(int sockfd, char *buf, int len) {
	FileDataFilter * filter =
			FileDataFilterManager::GetInstance()->getFileFilterBySockfd(sockfd);
	if (filter) {
		filter->addData((unsigned char *) buf, (unsigned int) len);
		if (filter->isFinish()) {
			unsigned int target_id = filter->getTargetId();
			FileRender *fileRender = filter->getFileRender();
			Connection *conn =
					ConnectionManager::GetInstance()->getConnectionById(
							target_id);
			if (!conn || !fileRender) {
				LOG_DEBUG("conn or fileRender is not found");
			} else {
				if (0 == fileRender->m_rw_type) { // read file form TV
					if (0 == fileRender->m_file_type) {
						VncDataPackage package;
						package.setSourceAndTarget(SERVER_ID, target_id);
						package.setCommandId(0x00000012, 0);
						std::string file_path = FILE_PATH
								+ fileRender->m_filename;
						package.setStringParam(file_path);
						std::string returnbuf = package.getProtocolBuffer();

						this->writeMsgToWebsocket(conn->getReadSockfd(),
								returnbuf.c_str(), returnbuf.length());
					} else if (1 == fileRender->m_file_type) { //picture form TV
						VncDataPackage package;
						package.setSourceAndTarget(SERVER_ID, target_id);
						package.setCommandId(0x00000010, 0);
						std::string file_path = FILE_PATH
								+ fileRender->m_filename;
						package.setStringParam(file_path);
						std::string returnbuf = package.getProtocolBuffer();

						this->writeMsgToWebsocket(conn->getReadSockfd(),
								returnbuf.c_str(), returnbuf.length());

					} else if (2 == fileRender->m_file_type) { //log file
						VncDataPackage package;
						package.setSourceAndTarget(SERVER_ID, target_id);
						package.setCommandId(0x00000014, 0);
						std::string file_path = FILE_PATH
								+ fileRender->m_filename;
						package.setStringParam(file_path);
						std::string returnbuf = package.getProtocolBuffer();

						this->writeMsgToWebsocket(conn->getReadSockfd(),
								returnbuf.c_str(), returnbuf.length());
					}
				} else if (1 == fileRender->m_rw_type) { // write file form TV

				}
			}
		}
	} else {

	}
	return 0;
}

int FileSocketServer::packageDataPackage(int sockfd, char *buf, int len) {
	return 0;
}

int FileSocketServer::dealTimeOut() {
	return 0;
}
