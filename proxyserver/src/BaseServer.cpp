/*
 * BaseServer.cpp
 *
 *  Created on: Jan 13, 2016
 *      Author: hwj
 */

#include "BaseServer.h"

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#include "ConnectInfo.h"
#include "MessageBuffer.h"
#include "logger.h"
#include "sys_def.h"

//BaseServer::ConnectionsMap BaseServer::m_conn_map;
BaseServer::PCConnMaps BaseServer::m_pcconn_map;
BaseServer::TvConnMaps BaseServer::m_tvconn_map;
BaseServer::MessageMaps BaseServer::m_message_map;

BaseServer::BaseServer(int port) :
		m_socket(-1), m_port(port) {
	init();
}

BaseServer::~BaseServer() {

}

void BaseServer::setnonblocking(int fd) {
	int opts;
	opts = fcntl(fd, F_GETFL);
	if (opts < 0) {
		LOG_DEBUG("fcntl(F_GETFL)");
		exit(1);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(fd, F_SETFL, opts) < 0) {
		LOG_DEBUG("fcntl(F_SETFL)");
		exit(1);
	}
}

int BaseServer::init() {
	int on = 1;
	struct sockaddr_in local;
	struct epoll_event ev;

	if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		LOG_DEBUG("socket ERROR!");
		return -1;
	}
	signal(SIGPIPE, SIG_IGN);

	setnonblocking(m_socket);
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&local, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(m_port);
	if (bind(m_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
		LOG_DEBUG("bind ERROR");
		return -1;
	}
	if (listen(m_socket, 20) < 0) {
		LOG_DEBUG("listen ERROR");
		return -1;
	}
	m_epollfd = epoll_create(MAX_EVENTS);
	if (-1 == m_epollfd) {
		LOG_DEBUG("epoll_create ERROR");
		return -1;
	}

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = m_socket;
	if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_socket, &ev) == -1) {
		LOG_DEBUG("epoll_ctl: listen_sock ERROR");
		return -1;
	}
	return 0;
}

void BaseServer::process()
{
	while(true) {
		processEpollEvents();
		processMsg();
	}
}

void BaseServer::processEpollEvents() {

	struct epoll_event ev, events[MAX_EVENTS];
	int nfds, nread, conn_sock, i, addrlen, n;
	struct sockaddr_in remote;
	char buf[BUFF_SIZE];

 
	nfds = epoll_wait(m_epollfd, events, MAX_EVENTS, EPOLL_TIMEOUT);
	if (nfds == -1) {
		LOG_DEBUG("epoll_pwait ERROR");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < nfds; ++i) {
		int sockfd = events[i].data.fd;
		if (sockfd == m_socket) {
			while ((conn_sock = accept(m_socket,
					(struct sockaddr *) &remote, (socklen_t *) &addrlen))
					> 0) {
				setnonblocking(conn_sock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, conn_sock, &ev)
						== -1) {
					LOG_ERROR("epoll_ctl: add");
					exit(EXIT_FAILURE);
				}
				acceptConnect(conn_sock);
			}
			if (conn_sock == -1) {
				if (errno != EAGAIN && errno != ECONNABORTED
						&& errno != EPROTO && errno != EINTR)
					LOG_ERROR("accept errno = %d", errno);
			}
			continue;
		}
		if (events[i].events & EPOLLIN) {
			n = 0;
			nread = 0;
			bzero(buf, BUFF_SIZE);
			while ((nread = read(sockfd, buf + n, BUFF_SIZE - (n + 1))) > 0) {
				n += nread;
			}
			if (nread == -1&& errno != EAGAIN && errno != EWOULDBLOCK
			&& errno != EINTR) {
				LOG_ERROR("read error: sockfd = %d errno = %d", sockfd, errno);
				struct epoll_event event_del;
				event_del.data.fd = events[i].data.fd;
				event_del.events = events[i].events;
				epoll_ctl(m_epollfd, EPOLL_CTL_DEL, event_del.data.fd,
						&event_del);
				disconnect(sockfd);
			} else {
				if (0 == n) {
					LOG_ERROR("read 0 bytes : sockfd = %d, errno = %d", sockfd, errno);
					//TODO socket error
					disconnect(sockfd);
					continue;
				}
				receiverMsg(sockfd, buf, n);
			}
		}
		if (events[i].events & EPOLLOUT) {

		}
	}
}

int BaseServer::processMsg() {

	BaseServer::MessageMaps::iterator iter = m_message_map.begin();

	while (iter != m_message_map.end()) {
		if (-1 != iter->first && iter->second) {

			bool isRemove = false;
			MessageBuffer *messagebuffer = iter->second;
			if (messagebuffer->isDataBufferEmpty()) {
				isRemove = true;
			} else {
				int sockfd = iter->first;


				MessageBuffer::DataBuffer * databuffer;
				int nwrite = 0;
				while (NULL != (databuffer = messagebuffer->getDataBuffer())) {
					nwrite = write(sockfd, (const char *) databuffer->m_buf, databuffer->m_cache_size);

					if ((unsigned int) nwrite == databuffer->m_cache_size) {
						messagebuffer->removeDataBufferFont();
						//LOG_DEBUG("write over data buffer");
					} else {
						if (-1 == nwrite && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
							LOG_ERROR( "write error : socket = %d, errno = %d", sockfd, errno);
							//TODO
							isRemove = true;

						} else if ((unsigned int) nwrite < databuffer->m_cache_size && -1 != nwrite) {
							databuffer->resetDataBuffer(nwrite);
							//LOG_DEBUG("write data buffer length = %d", nwrite);
						}
						break;
					}
				}
			}
			if (!isRemove) {
				++iter;
				continue;
			} else {
				delete iter->second;
				m_message_map.erase(iter++);
			}
		} else {
			m_message_map.erase(iter++);
		}
	}
	return 0;
}

int BaseServer::writeMsg(int sockfd, const char *buf, int len) {

	if (isMessageBufferExist(sockfd)) {
		insertMessageBuffer(sockfd, (unsigned char *) (buf), len);
//		LOG_DEBUG("socket %d has message buffer", sockfd);
////		struct epoll_event event;
////		event.data.fd = sockfd;
////		event.events = EPOLLIN | EPOLLlOUT | EPOLLET;
////		epoll_ctl(m_epollfd, EPOLL_CTL_MOD, event.data.fd, &event);
//
//		MessageBuffer *messagebuffer = getMessageBuffer(sockfd);
//		if (messagebuffer && !messagebuffer->isDataBufferEmpty()) {
//			MessageBuffer::DataBuffer * databuffer;
//			int nwrite = 0;
//			while (NULL != (databuffer = messagebuffer->getDataBuffer())) {
//				nwrite = write(sockfd, (const char *) databuffer->m_buf, databuffer->m_cache_size);
//				LOG_DEBUG("write buffer bytes %d, buffer length %d, sockfd  %d", nwrite,
//						databuffer->m_cache_size, sockfd);
//				if ((unsigned int) nwrite == databuffer->m_cache_size) {
//					messagebuffer->removeDataBufferFont();
//					LOG_DEBUG("write over buffer : sockfd = %d, buffer length = %d", sockfd, nwrite);
//				} else {
//					if (-1 == nwrite && errno != EAGAIN) {
//						LOG_DEBUG("write error : errno = %d", errno);
//						disconnectByTVSockfd(sockfd);
//
//					} else if ((unsigned int) nwrite < databuffer->m_cache_size
//							&& -1 != nwrite) {
//						databuffer->resetDataBuffer(nwrite);
//						//LOG_DEBUG("write buffer length %d,sockfd = %d", nwrite, sockfd);
//					}
//					break;
//				}
//
//			}
//		}
	} else {

		int ret = write(sockfd, buf, len);
		int writelen = 0;
		if (ret == -1) {
			if (errno != EAGAIN) {
				LOG_ERROR("write error : sockfd = %d, errno = %d", sockfd, errno);
				disconnectByTVSockfd(sockfd);
				struct epoll_event event;
				event.data.fd = sockfd;
				event.events = EPOLLIN | EPOLLET;
				epoll_ctl(m_epollfd, EPOLL_CTL_DEL, event.data.fd, &event);

				return ret;
			} else {
				writelen = 0;
			}
		} else {
			writelen = ret;
		}

		//LOG_DEBUG("write msg length %d, msg length %d, sockfd = %d", writelen, len, sockfd);
		if (len > writelen) {
			insertMessageBuffer(sockfd, (unsigned char *) (buf + writelen), len - writelen);
			//LOG_DEBUG("insertMessageBuffer sockfd = %d", sockfd);
		}

	}
	return 0;
}

int BaseServer::writeMsgToWebSocket(int sockfd, const char *buf, int len) {
	char web_buf[BUFF_SIZE];
	bzero(web_buf, BUFF_SIZE);

	packageMsgForWebsocket(web_buf, (char *) buf, &len);

	int ret = writeMsg(sockfd, web_buf, len);
	return ret;
}

int BaseServer::insertPCConn(unsigned int id, int sockfd)
{
	BaseManager::Locker lock(this);
	m_pcconn_map[id] = sockfd;
	return 0;
}

int BaseServer::getPCConnByTVId(unsigned int id)
{
	BaseManager::Locker lock(this);
	PCConnMaps::iterator iter = m_pcconn_map.find(id);
	if (m_pcconn_map.end() != iter) {
		//LOG_DEBUG("find PC conn sockfd by id 0x%08x success.", id);
		return iter->second;
	}
	LOG_ERROR("find PC conn sockfd by id 0x%08x fail.", id);
	return -1;
}

int BaseServer::removePCConnBySock(int sockfd)
{
	BaseManager::Locker lock(this);
	PCConnMaps::iterator iter = m_pcconn_map.begin();
	while (m_pcconn_map.end() != iter) {
		if (iter->second == sockfd) {
			m_pcconn_map.erase(iter++);
			//LOG_DEBUG("remove PC conn  by sockfd %d success.", sockfd);
			return 0;
		}
		++iter;
	}
	LOG_ERROR("remove PC conn  by sockfd %d fail.", sockfd);
	return -1;
}

int BaseServer::insertTVConn(int sockfd, ConnectInfo *info)
{
	BaseManager::Locker lock(this);
	if (!info) return -1;
	m_tvconn_map[sockfd] = info;
	return 0;
}

ConnectInfo * BaseServer::getConnInfoByTVSock(int sockfd)
{
	BaseManager::Locker lock(this);
	TvConnMaps::iterator iter = m_tvconn_map.find(sockfd);
	if (m_tvconn_map.end() != iter) {
		//LOG_DEBUG("find PC conn info by tv sockfd %d success.", sockfd);
		return iter->second;
	}
	LOG_ERROR("find PC conn info by tv sockfd %d fail.", sockfd);
	return NULL;
}

int BaseServer::getPCSockByTVSock(int sockfd)
{
	BaseManager::Locker lock(this);
	TvConnMaps::iterator iter = m_tvconn_map.find(sockfd);
	if (m_tvconn_map.end() != iter) {
		if (iter->second) {
			//LOG_DEBUG("find PC conn sockfd by tv sockfd %d success.", sockfd);
			return iter->second->getPCSockfd();
		}
	}
	LOG_ERROR("find PC conn sockfd by tv sockfd %d fail.", sockfd);
	return -1;
}

int BaseServer::removeTVConnBySock(int sockfd)
{
	BaseManager::Locker lock(this);
	TvConnMaps::iterator iter = m_tvconn_map.find(sockfd);
	if (m_tvconn_map.end() != iter) {
		if (iter->second) {
			delete iter->second;
			iter->second = NULL;
			//LOG_DEBUG("remove TV conn sockfd by tv sockfd %d success.", sockfd);
		}
		m_tvconn_map.erase(iter++);
		return 0;
	}
	LOG_ERROR("remove TV conn sockfd by tv sockfd %d success.", sockfd);
	return -1;
}

int BaseServer::packageMsgForWebsocket(char *dest, char *src, int *len) {
	if (!dest || !src || !len) {
		return -1;
	}
	int length = *len;

	dest[0] = 0x82;
	int len_pos = 2;

	if (length >= 126) {
		dest[1] = 126;
		dest[2] = (length >> 8) & 0xFF;
		dest[3] = (length) & 0xFF;
		len_pos += 2;
	} else {
		dest[1] = length & 0x7f;
	}

	memcpy(dest + len_pos, src, length);
	*len += len_pos;
	return 0;
}

int BaseServer::insertMessageBuffer(int sockfd, unsigned char *buf,
		unsigned int len) {
	MessageBuffer *buffer;
	BaseManager::Locker lock(this);

	MessageMaps::iterator iter = m_message_map.find(sockfd);
	if (m_message_map.end() == iter) {
		buffer = new MessageBuffer(sockfd);
		m_message_map[sockfd] = buffer;
		LOG_DEBUG("new  Message Buffer by socket = %d", sockfd);
	} else {
		buffer = iter->second;
	}
	if (buffer) {
		buffer->insertDataBuffer(buf, len);
	}

	LOG_DEBUG("insert Message Buffer by socket = %d", sockfd);

	return 0;
}

MessageBuffer *BaseServer::getMessageBuffer(int sockfd) {
	BaseManager::Locker lock(this);
	MessageMaps::iterator iter = m_message_map.find(sockfd);
	if (m_message_map.end() != iter) {
		return iter->second;
	}
	return NULL;
}

bool BaseServer::isMessageBufferExist(int sockfd) {
	BaseManager::Locker lock(this);
	MessageMaps::iterator iter = m_message_map.find(sockfd);
	if (m_message_map.end() != iter) {
		return true;
	}
	return false;
}

int BaseServer::disconnectByTVSockfd(int sockfd)
{
	BaseManager::Locker lock(this);
	TvConnMaps::iterator iter = m_tvconn_map.find(sockfd);
	if (m_tvconn_map.end() != iter) {
		if (iter->second) {
			int sockfd = iter->second->getPCSockfd();
			removePCConnBySock(sockfd);
			close(sockfd);
			delete iter->second;
			iter->second = NULL;
		}
		m_tvconn_map.erase(iter);
		LOG_DEBUG("remove Message Buffer by socket = %d", sockfd);
	}
	return 0;
}
