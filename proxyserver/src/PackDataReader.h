/*
 * PackDataReader.h
 *
 *  Created on: Dec 29, 2015
 *      Author: hwj
 */

#ifndef SRC_PACKDATAREADER_H_
#define SRC_PACKDATAREADER_H_

#include <queue>

class VncDataPackage;

class PackDataReader {
public:
	PackDataReader();
	virtual ~PackDataReader();
	int addBuffer(unsigned char *buf, unsigned int buffsize);
	bool getPackQueue(std::queue<VncDataPackage * > & queue);
	bool getPackQueue(VncDataPackage ** package);
private:
	unsigned char *	m_cache_buf;
    unsigned int 	m_cached_size;
    unsigned int 	m_alloc_size;
    unsigned int 	m_pkage_size;
    bool			m_getpkage;
    std::queue<VncDataPackage *> m_data;

};

#endif /* SRC_PACKDATAREADER_H_ */
