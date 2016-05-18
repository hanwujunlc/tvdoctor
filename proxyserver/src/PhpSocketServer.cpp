/*
 * PhpSocketServer.cpp
 *
 *  Created on: Dec 30, 2015
 *      Author: hwj
 */

#include "PhpSocketServer.h"
#include "TVListManager.h"
#include "TVOnlineListManager.h"
#include "Connection.h"
#include "ConnectionManager.h"

#include "sys_def.h"
#include "VncUtils.h"
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include "logger.h"

PhpSocketServer::PhpSocketServer(int port) :
		SocketServer(port), m_port(port), m_count(0), m_isFistStartTimer(false)

{

}

PhpSocketServer::~PhpSocketServer() {

}

int PhpSocketServer::acceptConnect(int sockfd) {
	LOG_DEBUG("phpsocket accepted sockfd = %d .", sockfd);
	return 0;
}

int PhpSocketServer::disconnect(int sockfd) {
	LOG_DEBUG("phpsocket disconnect,sockfd = %d .", sockfd);
	removeSockInEpoll(sockfd);
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	return 0;
}

void dealPhpSocketTimeout(int Num) {
	TVListManager::TVInfoMaps & tvinfomap =
			TVListManager::GetInstance()->getTVInfoMaps();
	if (!tvinfomap.empty()) {
		TVListManager::TVInfoMaps::iterator iter = tvinfomap.begin();
		while (tvinfomap.end() != iter) {

			TVListManager::TVInfoNode * info = iter->second;
			bool isRemove = false;
			if (info) {
				int oldtime = info->getTime();
				time_t nowtime = time(0);
				LOG_DEBUG("dealTimeOut nowtime= %d, oldtime= %d", (int )nowtime,
						oldtime);
				if (info->getTVid() != 0
						&& ((nowtime - oldtime) > PHP_TIMEOUT)) {
					int phpfd = info->getphpSockfd();
					if (-1 != phpfd) {
						int ret = write(phpfd, "timeout", 7);
						LOG_DEBUG("write  bytes %d msg to php, php id =%d.",
								ret, phpfd);
					}
					delete info;
					info = NULL;
					tvinfomap.erase(iter++);
					isRemove = true;
					LOG_DEBUG("m_tvinfo_map remove php id =%d success.", phpfd);

				}
			} else {
				tvinfomap.erase(iter++);
				isRemove = true;
			}
			if (!isRemove) {
				++iter;
			}
		}
	}
}

//int PhpSocketServer::receiverMsg(int sockfd, char *buf, int len) {
//	//std:string    recivedata = buf;
//	LOG_DEBUG("m_bufParam id = %s ,sockfd = %d", buf, sockfd);
//	unsigned int crc32_id = crc32_hash((const unsigned char*) (buf), len);
//	LOG_DEBUG("crc32_id =  0x%08x", crc32_id);
//
//	bool inOnline = TVOnlineListManager::GetInstance()->isTVinfoExist(crc32_id);
//
//
//	if (inOnline) {
//
//	} else {
//		bool pcIsExist = TVListManager::GetInstance()->isTVinfoExist(crc32_id,
//					TVListManager::TVInfoNode::DEVICES_PC);
//		if (pcIsExist) {
//
//			int phpsck = TVListManager::GetInstance()->getPhpSock(crc32_id);
//			LOG_DEBUG("pc reconnect the tv , php sockfd = %d", phpsck);
//			if (-1 != phpsck) {
//				writeMsgToPhpSocket(phpsck, "replace", 7);
//				removeSockInEpoll(phpsck);
//
//				shutdown(phpsck, SHUT_RDWR);
//				close(phpsck);
//			}
//			TVListManager::GetInstance()->removeTvinfo(crc32_id);
//
//		} else {
//			bool tvidIsexist = TVListManager::GetInstance()->isTVinfoExist(crc32_id, TVListManager::TVInfoNode::DEVICES_TV);
//			if (tvidIsexist) { //存在 通知phpTV接受请求了
//				bool isRefue = TVListManager::GetInstance()->getIsRefuse(
//						crc32_id);
//				if (isRefue) {
//					LOG_DEBUG("TV refuse the request, php  sockfd = %d",
//							sockfd);
//					writeMsgToPhpSocket(sockfd, "refuse", 6);
//				} else {
//					LOG_DEBUG("TV request is ok, php sockfd = %d", sockfd);
//					writeMsgToPhpSocket(sockfd, "ok", 2);
//					TVOnlineListManager::GetInstance()->insertTVinfo(crc32_id,
//							TVOnlineListManager::TVInfoNode::DEVICES_PC);
//				}
//				TVListManager::GetInstance()->removeTvinfo(crc32_id);
//				removeSockInEpoll(sockfd);
//				shutdown(sockfd, SHUT_RDWR);
//				close(sockfd);
//				return 0;
//			}
//		}
//	}
//
//	if (!m_isFistStartTimer) {
//		m_isFistStartTimer = true;
//		struct itimerval tick;
//		tick.it_interval.tv_sec = 3;
//		tick.it_interval.tv_usec = 0;
//		tick.it_value.tv_sec = 3;
//		tick.it_value.tv_usec = 0;
//		signal(SIGALRM, dealPhpSocketTimeout);
//		int ret = setitimer(ITIMER_REAL, &tick, NULL);
//		LOG_DEBUG("set timer  ret=%d", ret);
//	}
//
//	time_t t = time(0);
//	LOG_DEBUG("insert TVinfo time=%d", (int )t);
//	TVListManager::GetInstance()->insertTVinfo(crc32_id,
//			TVListManager::TVInfoNode::DEVICES_PC, false, sockfd, t);
//	return 0;
//}

int PhpSocketServer::receiverMsg(int sockfd, char *buf, int len) {
	//std:string    recivedata = buf;
	LOG_DEBUG("m_bufParam id = %s ,sockfd = %d", buf, sockfd);
	unsigned int crc32_id = crc32_hash((const unsigned char*) (buf), len);
	LOG_DEBUG("crc32_id =  0x%08x", crc32_id);

	bool inOnline = TVOnlineListManager::GetInstance()->isTVinfoExist(crc32_id);

	if (inOnline) {
		writeMsgToPhpSocket(sockfd, "exist", 5);
		removeSockInEpoll(sockfd);
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);

	} else {
		bool pcIsExist = TVListManager::GetInstance()->isTVinfoExist(crc32_id, TVListManager::TVInfoNode::DEVICES_PC);
		if (pcIsExist) {
			int phpsck = TVListManager::GetInstance()->getPhpSock(crc32_id);
			LOG_DEBUG("pc reconnect the tv , php sockfd = %d", phpsck);
			if (-1 != phpsck) {
				writeMsgToPhpSocket(phpsck, "replace", 7);
				removeSockInEpoll(phpsck);

				shutdown(phpsck, SHUT_RDWR);
				close(phpsck);
			}
			//TVListManager::GetInstance()->removeTvinfo(crc32_id);
			time_t t = time(0);
			LOG_DEBUG("insert TVinfo time=%d", (int )t);
			TVListManager::GetInstance()->insertTVinfo(crc32_id, TVListManager::TVInfoNode::DEVICES_PC, false, sockfd, t);

		} else {
			bool tvidIsexist = TVListManager::GetInstance()->isTVinfoExist(crc32_id, TVListManager::TVInfoNode::DEVICES_TV);
			if (tvidIsexist) { //存在 通知phpTV接受请求了
				bool isRefue = TVListManager::GetInstance()->getIsRefuse(
						crc32_id);
				if (isRefue) {
					LOG_DEBUG("TV refuse the request, php  sockfd = %d",
							sockfd);
					writeMsgToPhpSocket(sockfd, "refuse", 6);
				} else {
					LOG_DEBUG("TV request is ok, php sockfd = %d", sockfd);
					writeMsgToPhpSocket(sockfd, "ok", 2);
				}
				TVListManager::GetInstance()->removeTvinfo(crc32_id);
				removeSockInEpoll(sockfd);
				shutdown(sockfd, SHUT_RDWR);
				close(sockfd);
				return 0;
			} else {
				time_t t = time(0);
				LOG_DEBUG("insert TVinfo time=%d", (int )t);
				TVListManager::GetInstance()->insertTVinfo(crc32_id, TVListManager::TVInfoNode::DEVICES_PC, false, sockfd, t);
			}
		}
	}

	if (!m_isFistStartTimer) {
		m_isFistStartTimer = true;
		struct itimerval tick;
		tick.it_interval.tv_sec = 3;
		tick.it_interval.tv_usec = 0;
		tick.it_value.tv_sec = 3;
		tick.it_value.tv_usec = 0;
		signal(SIGALRM, dealPhpSocketTimeout);
		int ret = setitimer(ITIMER_REAL, &tick, NULL);
		LOG_DEBUG("set timer  ret=%d", ret);
	}

	return 0;
}

int PhpSocketServer::sendMsg(int sockfd, const char *buf, int len) {
	return 0;
}

int PhpSocketServer::parseDataPackage(int sockfd, char *buf, int len) {
	return 0;
}

int PhpSocketServer::packageDataPackage(int sockfd, char *buf, int len) {
	return 0;
}

int PhpSocketServer::dealTimeOut() {
	return 0;
}

