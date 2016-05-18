/*
 * ConnectInfo.h
 *
 *  Created on: Jan 14, 2016
 *      Author: hwj
 */

#ifndef SRC_CONNECTINFO_H_
#define SRC_CONNECTINFO_H_

class ConnectInfo {
public:
	ConnectInfo(unsigned int id, int pc_sockfd, int tv_sockfd);
	virtual ~ConnectInfo();

	int getPCSockfd(void);
	int getTVSockfd(void);
	unsigned int getTVId(void);

private:
	unsigned int m_tvid;
	int m_pc_sockfd;
	int m_tv_ritefd;


};

#endif /* SRC_CONNECTINFO_H_ */
