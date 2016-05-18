/*
 * FileDataFilter.h
 *
 *  Created on: Jan 4, 2016
 *      Author: hwj
 */

#ifndef SRC_FILEDATAFILTER_H_
#define SRC_FILEDATAFILTER_H_

class FileRender;
class FileDataFilter {
public:
	FileDataFilter(int sockfd);
	virtual ~FileDataFilter();

	int addData(unsigned char *buf, unsigned int buffsize);
	FileRender *getFileRender(void);
	bool isFinish(void);
	unsigned int getTargetId(void);

private:
	unsigned char *	m_cache_buf;
    unsigned int 	m_cached_size;
    unsigned int 	m_alloc_size;
    unsigned int 	m_pkage_size;
    bool m_is_get_header;
    bool m_is_finish;
	int m_sockfd;
	FileRender *m_filerender;
};

#endif /* SRC_FILEDATAFILTER_H_ */
