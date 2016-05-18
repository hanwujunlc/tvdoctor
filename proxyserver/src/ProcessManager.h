/*
 * ProcessManager.h
 *
 *  Created on: Dec 14, 2015
 *      Author: hwj
 */

#ifndef SRC_PROCESSMANAGER_H_
#define SRC_PROCESSMANAGER_H_

#include <pthread.h>

class WebSocketServer;
class TcpSocketServer;
class PhpSocketServer;
class FileSocketServer;
class LogSocketServer;
class LogWebSockServer;

class ProcessManager {
public:
	class Locker
	  {
	  public:
	    Locker(){ lock_ = &(ProcessManager::GetInstance()->m_mutex_lock);  pthread_mutex_lock(lock_); }
	    ~Locker(){ pthread_mutex_unlock(lock_); }
	  private:
	    pthread_mutex_t *lock_;
	  };
	static ProcessManager *GetInstance();

	virtual ~ProcessManager();

	void processs();
private:
	ProcessManager();
	void startThread();
	static ProcessManager *m_instance;

	WebSocketServer *m_websocket;
	TcpSocketServer *m_tcpsocket;
    PhpSocketServer *m_phpsocket;
	FileSocketServer *m_filesocket;
	LogSocketServer *m_logsocket;
	LogWebSockServer *m_logwebsock;

    pthread_t m_php_thread;
	pthread_t m_tcp_thread;
	pthread_t m_websocket_thread;
	pthread_t m_file_thread;
	pthread_t m_log_socket_thread;
	pthread_t m_log_websock_thread;
	pthread_mutex_t m_mutex_lock;

};

#endif /* SRC_PROCESSMANAGER_H_ */
