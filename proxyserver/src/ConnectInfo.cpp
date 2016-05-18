/*
 * ConnectInfo.cpp
 *
 *  Created on: Jan 14, 2016
 *      Author: hwj
 */

#include "ConnectInfo.h"

ConnectInfo::ConnectInfo(unsigned int id, int pc_sockfd, int tv_sockfd) :
	m_tvid(id),
	m_pc_sockfd(pc_sockfd),
	m_tv_ritefd(tv_sockfd)

{

}

ConnectInfo::~ConnectInfo() {
}

int ConnectInfo::getPCSockfd(void)
{
	return m_pc_sockfd;
}

int ConnectInfo::getTVSockfd(void)
{
	return m_tv_ritefd;
}

unsigned int ConnectInfo::getTVId(void)
{
	return m_tvid;
}
