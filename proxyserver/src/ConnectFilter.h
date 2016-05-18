/*
 * ConnectFilter.h
 *
 *  Created on: Jan 4, 2016
 *      Author: hwj
 */

#ifndef SRC_CONNECTFILTER_H_
#define SRC_CONNECTFILTER_H_

class PackDataReader;

class ConnectFilter {
public:
	ConnectFilter(int sockfd);
	virtual ~ConnectFilter();
	PackDataReader *getPackDataReader();
private:
	int m_sockfd;
	PackDataReader *m_pack_reader;
};

#endif /* SRC_CONNECTFILTER_H_ */
