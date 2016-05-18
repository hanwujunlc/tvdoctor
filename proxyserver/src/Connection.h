/*
 * Connection.h
 *
 *  Created on: Dec 22, 2015
 *      Author: hwj
 */

#ifndef SRC_CONNECTION_H_
#define SRC_CONNECTION_H_

#include <string>

class Connection {
public:
	enum DEVICES{
		DEVICES_TV = 0,
		DEVICES_PC,
	};
	Connection(int sockfd, unsigned int id, DEVICES devices);
	virtual ~Connection();

	void setWriteSockfd(int sockfd);
	int getWriteSockfd(void);

	int getReadSockfd(void);
	unsigned int getReadId(void);
	void setWriteId(unsigned int id);
	unsigned int getWriteId(void);
	DEVICES getDevices(void);
private :

	int m_readfd;
	int m_writefd;
	unsigned int m_id;
	unsigned int m_writeId;
	DEVICES m_devices;
	//std::string m_id;

};

#endif /* SRC_CONNECTION_H_ */
