/*
 * BaseManager.h
 *
 *  Created on: Dec 23, 2015
 *      Author: hwj
 */

#ifndef SRC_BASEMANAGER_H_
#define SRC_BASEMANAGER_H_

#include <pthread.h>

class BaseManager {
public:
	class Locker {
	public:
		Locker(BaseManager *manager) {
			m_lock = &manager->m_mutex_lock;
			pthread_mutex_lock(m_lock);
		}
		~Locker() {
			pthread_mutex_unlock(m_lock);
		}
	private:
		pthread_mutex_t *m_lock;
	};

public:
	BaseManager();
	virtual ~BaseManager();

protected:
	pthread_mutex_t m_mutex_lock;
};

#endif /* SRC_BASEMANAGER_H_ */
