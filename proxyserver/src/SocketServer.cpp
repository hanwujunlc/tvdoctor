/*
 * SocketServer.cpp
 *
 *  Created on: Dec 15, 2015
 *      Author: hwj
 */

#include "SocketServer.h"

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#include "MessageManager.h"
#include "MessageBuffer.h"
#include "PackDataReader.h"
#include "VncDataPackage.h"
#include "Connection.h"
#include "ConnectionManager.h"
#include "ConnectFilter.h"
#include "ConnectFilterManager.h"
#include "TVListManager.h"
#include "TVOnlineListManager.h"
#include "logger.h"
#include "sys_def.h"
#include "CommandId.h"

SocketServer::SocketServer(int port) :
		m_socket(-1), m_port(port) {
	init();
}

SocketServer::~SocketServer() {
}

int SocketServer::socket_set_keepalive (int fd)
{
  int alive, idle, cnt, intv;

  /* Set: use keepalive on fd */
  alive = 1;
  if (setsockopt
      (fd, SOL_SOCKET, SO_KEEPALIVE, &alive,
       sizeof alive) != 0)
    {
	  LOG_DEBUG ("Set keepalive error: %s.\n", strerror (errno));
      return -1;
    }

  /* １０秒钟无数据，触发保活机制，发送保活包 */
  idle = 10;
  if (setsockopt (fd, SOL_TCP, TCP_KEEPIDLE, &idle, sizeof idle) != 0)
    {
	  LOG_DEBUG ("Set keepalive idle error: %s.\n", strerror (errno));
      return -1;
    }

  /* 如果没有收到回应，则５秒钟后重发保活包 */
  intv = 5;
  if (setsockopt (fd, SOL_TCP, TCP_KEEPINTVL, &intv, sizeof intv) != 0)
    {
	  LOG_DEBUG ("Set keepalive intv error: %s.\n", strerror (errno));
      return -1;
    }

  /* 连续３次没收到保活包，视为连接失效 */
  cnt = 3;
  if (setsockopt (fd, SOL_TCP, TCP_KEEPCNT, &cnt, sizeof cnt) != 0)
    {
	  LOG_DEBUG ("Set keepalive cnt error: %s.\n", strerror (errno));
      return -1;
    }

  return 0;
}

void SocketServer::setnonblocking(int fd) {
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

int SocketServer::init() {
	int on = 1;
	struct sockaddr_in local;
	struct epoll_event ev;

	if ((this->m_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		LOG_DEBUG("socket ERROR!");
		return -1;
	}
	signal(SIGPIPE, SIG_IGN);

	setnonblocking(this->m_socket);
	setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	socket_set_keepalive(this->m_socket);

	bzero(&local, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(this->m_port);
	if (bind(this->m_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
		LOG_DEBUG("bind ERROR");
		return -1;
	}
	if (listen(this->m_socket, 20) < 0) {
		LOG_DEBUG("listen ERROR");
		return -1;
	}
	this->m_epollfd = epoll_create(MAX_EVENTS);
	if (-1 == this->m_epollfd) {
		LOG_DEBUG("epoll_create ERROR");
		return -1;
	}

	ev.events = EPOLLIN;
	ev.data.fd = this->m_socket;
	if (epoll_ctl(this->m_epollfd, EPOLL_CTL_ADD, this->m_socket, &ev) == -1) {
		LOG_DEBUG("epoll_ctl: listen_sock ERROR");
		return -1;
	}
	return 0;
}

void SocketServer::process()
{
	while(true) {
		processEpollEvents();
		processMsg();
	}
}


void SocketServer::processEpollEvents() {
	struct epoll_event ev, events[MAX_EVENTS];
	int nfds, nread, conn_sock, i, addrlen, n;
	struct sockaddr_in remote;
	char buf[BUFF_SIZE];
	for (;;) {
		
		nfds = epoll_wait(this->m_epollfd, events, MAX_EVENTS, EPOLL_TIMEOUT);
		if (nfds == -1) {
			LOG_DEBUG("epoll_pwait ERROR");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < nfds; ++i) {
			int sockfd = events[i].data.fd;
			if (sockfd == this->m_socket) {
				while ((conn_sock = accept(this->m_socket,
						(struct sockaddr *) &remote, (socklen_t *) &addrlen))
						> 0) {
					setnonblocking(conn_sock);
					socket_set_keepalive(conn_sock);
					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = conn_sock;
					if (epoll_ctl(this->m_epollfd, EPOLL_CTL_ADD, conn_sock,
							&ev) == -1) {
						LOG_DEBUG("epoll_ctl: add");
						exit(EXIT_FAILURE);
					}
					acceptConnect(conn_sock);
				}
				if (conn_sock == -1) {
					if (errno != EAGAIN && errno != ECONNABORTED
							&& errno != EPROTO && errno != EINTR)
						LOG_DEBUG("accept");
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
				if (nread == -1 && errno != EAGAIN && errno != EWOULDBLOCK
				&& errno != EINTR) {
					LOG_DEBUG("read error: sockfd = %d, errno = %d", sockfd, errno);
					struct epoll_event event_del;
					event_del.data.fd = events[i].data.fd;
					event_del.events = events[i].events;
					epoll_ctl(this->m_epollfd, EPOLL_CTL_DEL, event_del.data.fd,
							&event_del);
					disconnect(sockfd);
				} else {
					if (0 == n) {
						LOG_DEBUG("read 0 bytes : sockfd = %d, errno = %d", sockfd, errno);
						//TODO socket error
						disconnect(sockfd);
						continue;
					}
					receiverMsg(sockfd, buf, n);
				}
			}
			if (events[i].events & EPOLLOUT) {

//				bool isError = false;
//				bool isDone = true;
//				MessageBuffer *messagebuffer =
//						MessageManager::GetInstance()->getMessageBuffer(sockfd);
//				if (messagebuffer && !messagebuffer->isDataBufferEmpty()) {
//					MessageBuffer::DataBuffer * databuffer;
//					int nwrite = 0;
//					while (NULL != (databuffer = messagebuffer->getDataBuffer())) {
//						nwrite = write(sockfd, (const char *) databuffer->m_buf,
//								databuffer->m_cache_size);
//
//						if ((unsigned int) nwrite == databuffer->m_cache_size) {
//							messagebuffer->removeDataBufferFont();
//						} else {
//							if (-1 == nwrite && errno != EAGAIN) {
//								LOG_DEBUG("write error : errno = %d", errno);
//								disconnectBySockfd(sockfd);
//								isError = true;
//							} else if ((unsigned int) nwrite
//									< databuffer->m_cache_size
//									&& -1 != nwrite) {
//								databuffer->resetDataBuffer(nwrite);
//							}
//							isDone = false;
//							break;
//						}
//
//					}
//				}
//
//				if (isError) {
//					struct epoll_event event;
//					event.data.fd = sockfd;
//					event.events = events[i].events;
//					epoll_ctl(this->m_epollfd, EPOLL_CTL_DEL, event.data.fd,
//							&event);
//				} else {
//					if (isDone) {
//						struct epoll_event event;
//						event.data.fd = sockfd;
//						event.events = EPOLLIN | EPOLLET;
//						epoll_ctl(this->m_epollfd, EPOLL_CTL_MOD, event.data.fd,
//								&event);
//					}
//				}
			}
		}
	}
}

int SocketServer::processMsg() {

	MessageManager::MessageMaps &message_map = MessageManager::GetInstance()->getMessageMap();
	MessageManager::MessageMaps::iterator iter = message_map.begin();
	while (iter != message_map.end()) {
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
					nwrite = write(sockfd, (const char *) databuffer->m_buf,
							databuffer->m_cache_size);

					if ((unsigned int) nwrite == databuffer->m_cache_size) {
						messagebuffer->removeDataBufferFont();
						//LOG_DEBUG("write over data buffer");
					} else {
						if (-1 == nwrite && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
							LOG_ERROR("write error : socket = %d, errno = %d", sockfd, errno);
							disconnectBySockfd(sockfd);
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
				message_map.erase(iter++);
			}
		} else {
			if (iter->second)
				delete iter->second;
			message_map.erase(iter++);
		}
	}
	return 0;
}

void SocketServer::handlEvent(int sockfd, char *buf, int len) {
	int ret = writeMsg(sockfd, buf, len);
	LOG_DEBUG("write to %d message is : %d", sockfd, ret);
}

int SocketServer::writeMsgToWebsocket(int sockfd, const char *buf, int len) {
	char web_buf[BUFF_SIZE];
	bzero(web_buf, BUFF_SIZE);

	packageMsgForWebsocket(web_buf, (char *) buf, &len);
	LOG_DEBUG("writeMsgToWebsocket send to %d MSG length =  %d", sockfd, len);
	int ret = writeMsg(sockfd, web_buf, len);

	return ret;
}

int SocketServer::writeMsgToTcpsocket(int sockfd, const char *buf, int len) {

	LOG_DEBUG("writeMsgToTcpsocket send to %d MSG length =  %d", sockfd, len);
	int ret = writeMsg(sockfd, buf, len);
	return ret;
}

int SocketServer::packageMsgForWebsocket(char *dest, char *src, int *len) {
	if (!dest || !src || !len) {
		return -1;
	}
	int length = *len;
	//LOG_DEBUG("length = %d, %02X", length, length);
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

	//LOG_DEBUG("dest[1] = %02X", dest[1] & 0xFF);
	memcpy(dest + len_pos, src, length);
	*len += len_pos;
	return 0;
}

int SocketServer::writeMsg(int sockfd, const char *buf, int len) {

	if (MessageManager::GetInstance()->isMessageBufferExist(sockfd)) {
		MessageManager::GetInstance()->insertMessageBuffer(sockfd, (unsigned char *) (buf), len);
		LOG_DEBUG("Socket %d has message buffer", sockfd);
//		struct epoll_event event;
//		event.data.fd = sockfd;
//		event.events = EPOLLIN | EPOLLOUT | EPOLLET;
//		epoll_ctl(this->m_epollfd, EPOLL_CTL_MOD, event.data.fd, &event);
		return 0;
	}

	int ret = write(sockfd, buf, len);
	int writelen = 0;
	if (ret == -1) {
		if (errno != EAGAIN) {
			LOG_DEBUG("write error : sockfd = %d, errno = %d,", sockfd, errno);
			disconnectBySockfd(sockfd);
			struct epoll_event event;
			event.data.fd = sockfd;
			event.events = EPOLLIN | EPOLLET;
			epoll_ctl(this->m_epollfd, EPOLL_CTL_DEL, event.data.fd, &event);

			return ret;
		} else {
			writelen = 0;
		}
	} else {
		writelen = ret;
	}

	if (len > writelen) {
		MessageManager::GetInstance()->insertMessageBuffer(sockfd,
				(unsigned char *) (buf + writelen), len - writelen);

//		struct epoll_event event;
//		event.data.fd = sockfd;
//		event.events = EPOLLIN | /*EPOLLOUT | */EPOLLET;
//		epoll_ctl(this->m_epollfd, EPOLL_CTL_MOD, event.data.fd, &event);
	}

	LOG_DEBUG("write socket to %d  writed message length is : %d, message length is %d ", sockfd, ret, len);
	return ret;
}

int SocketServer::removeSockInEpoll(int sockfd)
{
	struct epoll_event event;
	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(this->m_epollfd, EPOLL_CTL_DEL, event.data.fd, &event);
	return 0;
}

int SocketServer::disconnectBySockfd(int sockfd) {

	LOG_DEBUG("disconnectBySockfd sockfd = %d", sockfd);
	removeSockInEpoll(sockfd);
	ConnectFilterManager::GetInstance()->removeConnFilter(sockfd);
	MessageManager::GetInstance()->removeMessageBuffer(sockfd);

	Connection *conn = ConnectionManager::GetInstance()->getConnByReadSockfd(sockfd);
	if (conn) {
		unsigned int id = conn->getReadId();
		unsigned int wid = conn->getWriteId();
		LOG_DEBUG("disconnectBySockfd read id = 0x%08x, write id = 0x%08x", id, wid);
		unsigned int tv_id = 0;
		int wsockfd = conn->getWriteSockfd();
		LOG_DEBUG("disconnectBySockfd write sockfd = %d", wsockfd);

		if (0 != wid && -1 != wsockfd) {
			if (Connection::DEVICES_PC == conn->getDevices()) {
				//this->disconnectToWebSocket(sockfd, id);
				this->disconnectToTcpSocket(wsockfd, wid);
				tv_id = wid;
			} else if(Connection::DEVICES_TV == conn->getDevices()) {
				this->disconnectToWebSocket(wsockfd, wid);
				//this->disconnectToTcpSocket(sockfd, id);
				tv_id = id;
			}
			ConnectionManager::GetInstance()->removeConnection(wid);
			ConnectFilterManager::GetInstance()->removeConnFilter(wsockfd);
			MessageManager::GetInstance()->removeMessageBuffer(wsockfd);
			TVListManager::GetInstance()->removeTvinfo(tv_id);
			TVOnlineListManager::GetInstance()->removeTvinfo(tv_id);
		} else {
			if(Connection::DEVICES_TV == conn->getDevices()) {
				TVListManager::GetInstance()->removeTvinfo(id);
				TVOnlineListManager::GetInstance()->removeTvinfo(id);
			}

		}

		ConnectionManager::GetInstance()->removeConnection(id);

	}

	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);

	return 0;
}

int SocketServer::disconnectBySockfd(int sockfd, DISCONNECT_ENUM how)
{
	LOG_DEBUG("disconnectBySockfd sockfd = %d how = %d", sockfd, how);

	removeSockInEpoll(sockfd);
	ConnectFilterManager::GetInstance()->removeConnFilter(sockfd);
	MessageManager::GetInstance()->removeMessageBuffer(sockfd);

	Connection *conn = ConnectionManager::GetInstance()->getConnByReadSockfd(sockfd);

	if (conn) {
		unsigned int id = conn->getReadId();
		unsigned int wid = conn->getWriteId();
		LOG_DEBUG("disconnectBySockfd read id = 0x%08x, write id = 0x%08x", id, wid);
		int wsockfd = conn->getWriteSockfd();
		LOG_DEBUG("disconnectBySockfd write sockfd = %d", wsockfd);

		if (0 != wid && -1 != wsockfd) {

			switch (how) {
				case DISCONNECT_REPLACE: {

					disconnectToWebSocket(wsockfd, wid, how);

					shutdown(sockfd, SHUT_RDWR);
					close(sockfd);
				} break;
				default:
					break;
			}

			ConnectionManager::GetInstance()->removeConnection(wid);
			ConnectFilterManager::GetInstance()->removeConnFilter(wsockfd);
			MessageManager::GetInstance()->removeMessageBuffer(wsockfd);
		}

		ConnectionManager::GetInstance()->removeConnection(id);

	} else {
		LOG_DEBUG("can not find connection by sockfd = %d", sockfd);
		return -1;
	}

	return 0;
}

int SocketServer::disconnectToWebSocket(int sockfd, unsigned int id) {
	LOG_DEBUG("disconnect WebSocket socket = %d id = 0x%08x", sockfd, id);
	VncDataPackage package;
	package.setSourceAndTarget(SERVER_ID, id);
	package.setCommandId(CMD_NOTIFY_TV_OFFLINE, 0);
	std::string returnbuf = package.getProtocolBuffer();

	writeMsgToWebsocket(sockfd, returnbuf.c_str(), returnbuf.length());
	removeSockInEpoll(sockfd);
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	return 0;
}

int SocketServer::disconnectToWebSocket(int sockfd, unsigned int id, DISCONNECT_ENUM how)
{
	LOG_DEBUG("disconnect WebSocket socket = %d id = 0x%08x, how = %d", sockfd, id, how);
	switch (how) {
	case DISCONNECT_REPLACE: {
		VncDataPackage package;
		package.setSourceAndTarget(SERVER_ID, id);
		package.setCommandId(CMD_ANOTHER_PC_CTRL, 0);
		std::string returnbuf = package.getProtocolBuffer();
		writeMsgToWebsocket(sockfd, returnbuf.c_str(), returnbuf.length());
	}
		break;
	default:
		break;
	}
	removeSockInEpoll(sockfd);
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	return 0;
}

int SocketServer::disconnectToTcpSocket(int sockfd, unsigned int id) {
	LOG_DEBUG("disconnect TcpSocket socket = %d id = 0x%08x", sockfd, id);
	VncDataPackage package;
	package.setSourceAndTarget(SERVER_ID, id);
	package.setCommandId(T2S_CMD_NOTIFY_PC_OFFLINE, 0);

	std::string returnbuf = package.getProtocolBuffer();

	writeMsgToTcpsocket(sockfd, returnbuf.c_str(), returnbuf.length());
	removeSockInEpoll(sockfd);
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	return 0;
}

int SocketServer::writeMsgToPhpSocket(int sockfd, const char *buf, int len) {
	LOG_DEBUG("writeMsgToPhpSocket socket id %d message length is : %d", sockfd,
			len);
	int ret = write(sockfd, buf, len);
	if (ret == -1 && errno != EAGAIN) {
		LOG_DEBUG("write error : errno = %d", errno);
		return ret;
	}
	LOG_DEBUG("writeMsgToPhpSocket  socket id %d message is : %d", sockfd, ret);
	return ret;
}
