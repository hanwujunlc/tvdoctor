/*
 * BaseManager.cpp
 *
 *  Created on: Dec 23, 2015
 *      Author: hwj
 */

#include "BaseManager.h"

BaseManager::BaseManager()
{
	// TODO Auto-generated constructor stub
	pthread_mutex_init(&m_mutex_lock, 0);
}

BaseManager::~BaseManager() {
	// TODO Auto-generated destructor stub
	pthread_mutex_destroy(&m_mutex_lock);
}

