/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *  Copyright (C) 2010  The Mana Development Team
 *
 *  This file is part of The Mana Server.
 *
 *  The Mana Server is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana Server is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana Server.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "logger.h"
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <iosfwd>
#include <iomanip>

#include "../sys_def.h"

Logger *Logger::m_instance = new Logger();

Logger::Logger() :
		mVerbosity(Logger::Debug),
		m_filefd (-1),
		mHasTimestamp(true),
		mTeeMode(true),
		mLogRotation(false),
		mMaxFileSize(1024),
		mSwitchLogEachDay(true),
		m_file_size(0)
{

}

Logger::~Logger()
{

}

Logger *Logger::GetInstance(void)
{
	return m_instance;

}

void LOG__(const char *format, ...) {

	char buf[1024];
	va_list  args;
	va_start(args, format);
 	vsprintf(buf, format, args);
 	va_end(args);
 	printf("File: " __FILE__ ", Line: %03d: %s\n", __LINE__, buf);
}

void Logger::LOG(Level verbosity, const char *format, ...)
{
	char buf[1024];
	va_list  args;
	va_start(args, format);
 	vsprintf(buf, format, args);
 	va_end(args);
 	//printf("File: "__FILE__", Line: %03d: %s\n", __LINE__, buf);
 	Logger::GetInstance()->output(verbosity, buf);

}

/**
  * Gets the current time.
  *
  * @return the current time as string.
  */
static std::string getCurrentTime()
{
    time_t now;
    tm local;

    // Get current time_t value
    time(&now);

    // Convert time_t to tm struct to break the time into individual
    // constituents.
    local = *(localtime(&now));

    // Stringify the time, the format is: hh:mm:ss
    using namespace std;
    ostringstream os;
    os << setw(2) << setfill('0') << local.tm_hour
       << ":" << setw(2) << setfill('0') << local.tm_min
       << ":" << setw(2) << setfill('0') << local.tm_sec;

    return os.str();
}

/**
  * Gets the current date.
  *
  * @return the current date as string.
  */
static std::string getCurrentDate()
{
    time_t now;
    tm local;

    // Get current time_t value
    time(&now);

    // Convert time_t to tm struct to break the time into individual
    // constituents.
    local = *(localtime(&now));

    // Stringify the time, the format is: yyyy-mm-dd
    using namespace std;
    ostringstream os;
    os << setw(4) << setfill('0') << (local.tm_year + 1900)
       << "-" << setw(2) << setfill('0') << local.tm_mon + 1
       << "-" << setw(2) << setfill('0') << local.tm_mday;

    return os.str();
}

/**
  * Check whether the day has changed since the last call.
  *
  * @return whether the day has changed.
  */
static bool getDayChanged()
{
    std::string dayDate = getCurrentDate();

    if (Logger::GetInstance()->mLastCallDate != dayDate)
    {
        // Keep track of the old date.
    	Logger::GetInstance()->mOldDate = Logger::GetInstance()->mLastCallDate;
        // Reset the current date for next call.
    	Logger::GetInstance()->mLastCallDate = dayDate;
        return true;
    }
    return false;
}

void Logger::initialize(const std::string &logFile)
{

    setLogFile(logFile, true);

    LOG_INFO("logFile = %s", logFile.c_str());
    // Write the messages to both the screen and the log file.
    setTeeMode(false);
    //LOG_INFO("Using log file: " << logFile);
    //LOG_INFO("Using log file: %s", logFile.c_str());

    // Set up the options related to log rotation.
    setLogRotation(true);
    setMaxLogfileSize(DEFAULT_LOG_MAXSIZE);
    setSwitchLogEachDay(true);
}

void Logger::setLogFile(const std::string &logFile, bool append)
{
    // Close the current log file.

    // Open the file for output
    // and remove the former file contents depending on the append flag.
    m_filefd = open((DEFAULT_LOG_PATH + logFile).c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (-1 == m_filefd) {
    	perror("open()");
    	return;
    }
    mFilename = logFile;

    mLastCallDate = mOldDate = getCurrentDate();

}

void Logger::output(Level verbosity, const char *buf) {
	outToFile(verbosity, buf);
	switchLogs();
}


void Logger::outToFile(Level verbosity, const char *buf)
{
	static const char *prefixes[] = { "[FTL]", "[ERR]", "[WRN]", "[INF]",
			"[DBG]" };

	char data[1024];
	bzero(data, 1024);
	sprintf(data, "[");

	//write(this->m_filefd, "[", 1);
	std::string current = getCurrentTime();
	//write(this->m_filefd, current.c_str(), current.length());
	sprintf(data + 1, "%s", current.c_str());
	sprintf(data + 1 + current.length(), "] ");
	sprintf(data + 1 + current.length() + 2, "%s ", prefixes[verbosity]);
	sprintf(data + 1 + current.length() + 2 + 6, "%s", buf);

	if (LOG_DEBUG_LEVEL >= verbosity) {
//		write(this->m_filefd, data, strlen(data));
//		m_file_size += strlen(data);
		writeLog(data);
	}
}

void Logger::writeLog(const char *format, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	write(this->m_filefd, buf, strlen(buf));
	m_file_size += strlen(buf);
	if (mTeeMode) {
		printf(buf);
	}
}

void Logger::switchLogs()
{
    // Handles logswitch if enabled
    // and if at least one switch condition permits it.
    if (!mLogRotation || (mMaxFileSize <= 0 && !mSwitchLogEachDay))
        return;

    // Update current filesize
    long fileSize = m_file_size;

    bool dayJustChanged = getDayChanged();

    if ((fileSize >= (mMaxFileSize * 1024))
        || (mSwitchLogEachDay && dayJustChanged))
    {
        // Close logfile, rename it and open a new one
    	close(m_filefd);
        // Stringify the time, the format is: path/yyyy-mm-dd-n_logFilename.
        using namespace std;
        ostringstream os;
        os << (dayJustChanged ? mOldDate : getCurrentDate());

        os << "_" << getCurrentTime();

		std::string newFileName = DEFAULT_LOG_PATH +  os.str() + "_" + mFilename;

        if (rename((DEFAULT_LOG_PATH + mFilename).c_str(), newFileName.c_str()) == 0)
        {
            setLogFile(mFilename, true);
            writeLog("change old logfile %s, new logFile = %s\n", newFileName.c_str(), mFilename.c_str());
            m_file_size = 0;
        }
        else
        {
            // Keep the logging after emptying the original log file.
            setLogFile(mFilename);
            writeLog("change logFile error ! %s\n", mFilename.c_str());
        }
    }
}
