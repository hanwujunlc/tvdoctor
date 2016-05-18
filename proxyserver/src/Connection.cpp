/*
 * Connection.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: hwj
 */

#include "Connection.h"

Connection::Connection(int sockfd, unsigned int id, DEVICES devices) :
	m_readfd(sockfd),
	m_writefd(-1),
	m_id(id),
	m_writeId(0),
	m_devices(devices)
{

}

Connection::~Connection() {
}

void Connection::setWriteSockfd(int sockfd)
{
	this->m_writefd = sockfd;
}

int Connection::getWriteSockfd(void)
{
	return this->m_writefd;
}

int Connection::getReadSockfd(void)
{
	return this->m_readfd;
}

unsigned int Connection::getReadId(void)
{
	return this->m_id;
}

void Connection::setWriteId(unsigned int id)
{
	this->m_writeId = id;
}
unsigned int Connection::getWriteId(void)
{
	return this->m_writeId;
}

Connection::DEVICES Connection::getDevices(void)
{
	return this->m_devices;
}


