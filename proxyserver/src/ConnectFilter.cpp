/*
 * ConnectFilter.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: hwj
 */

#include "ConnectFilter.h"
#include "PackDataReader.h"

ConnectFilter::ConnectFilter(int sockfd) :
	m_sockfd(sockfd)
{
	m_pack_reader = new PackDataReader();
}

ConnectFilter::~ConnectFilter() {
	delete m_pack_reader;
}

PackDataReader *ConnectFilter::getPackDataReader()
{
	return m_pack_reader;
}

