/*
 * MessageManager.h
 *
 *  Created on: Dec 17, 2015
 *      Author: hwj
 */

#ifndef SRC_MESSAGEMANAGER_H_
#define SRC_MESSAGEMANAGER_H_

#include "BaseManager.h"

#include <map>

class MessageBuffer;
class MessageManager : public BaseManager{
public:
	static MessageManager *GetInstance();
	virtual ~MessageManager();
	int insertMessageBuffer(int sockfd, unsigned char *buf, unsigned int len);
	int removeMessageBuffer(int sockfd);

	MessageBuffer *getMessageBuffer(int sockfd);
	bool isMessageBufferExist(int sockfd);
private:
	MessageManager();

	static MessageManager *m_instance;

	typedef std::map<int, MessageBuffer * > MessageMaps;
	MessageMaps m_message_map;
};

#endif /* SRC_MESSAGEHANDLER_H_ */
