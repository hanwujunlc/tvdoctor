/*
 * FileRender.h
 *
 *  Created on: Dec 31, 2015
 *      Author: hwj
 */

#ifndef SRC_FILERENDER_H_
#define SRC_FILERENDER_H_

#include <string>

class FileRender {
public:
	FileRender(int sockfd, std::string filename, int rw_type, int file_type);
	virtual ~FileRender();

	int writeToFile(unsigned char *buf, unsigned int len);
	int readFromFile(unsigned char *buf, unsigned int len);

	int writeMsgToTV(int sockfd, unsigned char *buf, unsigned int len);
	int writeMsg(int sockfd, unsigned char *buf, unsigned int len);
	int writeFileToTV(int sockfd);

	unsigned long getFileSize();
//private:
	std::string m_filename;
	unsigned int m_source_id;
	unsigned int m_target_id;
	int m_filefd;
	int m_sockfd;
	int m_file_size;
	int m_data_size; //read file data size;
	int m_rw_type; // 0 is read, 1 is write;
	int m_file_type;
};

#endif /* SRC_FILERENDER_H_ */
