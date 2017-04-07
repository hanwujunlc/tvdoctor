/*
 * ProcessManager.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: hwj
 */

#include "ProcessManager.h"
#include "WebSocketServer.h"
#include "TcpSocketServer.h"
#include "PhpSocketServer.h"
#include "FileSocketServer.h"
#include "LogSocketServer.h"
#include "LogWebSockServer.h"

#include "sys_def.h"
#include "logger.h"

#include <unistd.h>

ProcessManager *ProcessManager::m_instance = new ProcessManager();

ProcessManager *ProcessManager::GetInstance() {
	return m_instance;
}

ProcessManager::ProcessManager() :
		m_websocket(NULL),
		m_tcpsocket(NULL),
		m_phpsocket(NULL),
		m_filesocket(NULL),
		m_logsocket(NULL),
		m_logwebsock(NULL)
{
	pthread_mutex_init(&m_mutex_lock, 0);
	startThread();
}

ProcessManager::~ProcessManager() {

	delete m_websocket;
	m_websocket = NULL;
	delete m_tcpsocket;
	m_tcpsocket = NULL;
	delete m_phpsocket ;
	m_phpsocket = NULL;

    if (m_php_thread != 0)
    {
    	pthread_join(m_php_thread, 0);
		m_php_thread = 0;
    }

	if (m_tcp_thread != 0) {
		pthread_join(m_tcp_thread, 0);
		m_tcp_thread = 0;
	}
	if (m_websocket_thread != 0) {
		pthread_join(m_websocket_thread, 0);
		m_websocket_thread = 0;
	}
	pthread_mutex_destroy(&m_mutex_lock);
}

void ProcessManager::processs() {

	while(1)
	{
		sleep(1000);
	}
}

static void *start_tcp_connect(void *arg) {
	TcpSocketServer *tcpsocket = (TcpSocketServer *) arg;
	tcpsocket = new TcpSocketServer(TCPSOCK_PORT);
	tcpsocket->process();
	return NULL;
}

static void *start_websocket_server(void *arg) {
	WebSocketServer *webserver = (WebSocketServer *) arg;
	webserver = new WebSocketServer(WEBSOCK_PORT);
	webserver->process();
	return NULL;
}

static void *start_phpsocket_server(void *arg) {
	PhpSocketServer *phpserver = (PhpSocketServer *) arg;
	phpserver = new PhpSocketServer(PHPSOCK_PORT);
	phpserver->process();
	return NULL;
}


static void *start_filesocket_server(void *arg) {
	FileSocketServer *fileserver = (FileSocketServer *) arg;
	fileserver = new FileSocketServer(FILESOCK_PORT);
	fileserver->process();
	return NULL;
}

static void *start_logsocket_server(void *arg) {
	LogSocketServer *logserver = (LogSocketServer *) arg;
	logserver = new LogSocketServer(LOGSOCK_PORT);
	logserver->process();
	return NULL;
}

static void *start_logwebsock_server(void *arg) {
	LogWebSockServer *logwebserver = (LogWebSockServer *) arg;
	logwebserver = new LogWebSockServer(LOGWEBSOCK_PORT);
	logwebserver->process();
	return NULL;
}

void ProcessManager::startThread() {
	std::string logfile = DEFAULT_LOG_FILE;
	Logger::GetInstance()->initialize(logfile);

	pthread_create(&m_php_thread, NULL, start_phpsocket_server, m_phpsocket);
	pthread_create(&m_tcp_thread, NULL, start_tcp_connect, m_tcpsocket);
	pthread_create(&m_websocket_thread, NULL, start_websocket_server, m_websocket);
	pthread_create(&m_file_thread, NULL, start_filesocket_server, m_filesocket);
	pthread_create(&m_log_socket_thread, NULL, start_logsocket_server, m_logsocket);
	pthread_create(&m_log_websock_thread, NULL, start_logwebsock_server, m_logwebsock);
}

