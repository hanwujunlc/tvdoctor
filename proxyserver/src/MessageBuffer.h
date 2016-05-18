/*
 * MessageBuffer.h
 *
 *  Created on: Dec 23, 2015
 *      Author: hwj
 */

#ifndef SRC_MESSAGEBUFFER_H_
#define SRC_MESSAGEBUFFER_H_

#include <queue>

class MessageBuffer {
public:
	class DataBuffer {
	public:
		DataBuffer(unsigned char *buf, unsigned int len);
		virtual ~DataBuffer();
		int resetDataBuffer(unsigned int len);

		unsigned char *m_buf;
		unsigned int m_alloc_size;
		unsigned int m_cache_size;
		unsigned int m_pos_size;
	};
	MessageBuffer(int sockfd);
	virtual ~MessageBuffer();

	void insertDataBuffer(unsigned char *buf, unsigned int len);
	void insertDataBuffer(DataBuffer *buffer);
	DataBuffer *getDataBuffer(void);
	int removeDataBufferFont(void);
	bool isDataBufferEmpty(void);
	int getDataBufferSize(void);
private:
	typedef std::queue<DataBuffer *> DataBuffQues;
	DataBuffQues m_data_que;

};

#endif /* MESSAGEBUFFER_H_ */
