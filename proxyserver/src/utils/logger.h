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

#ifndef LOGGER_H
#define LOGGER_H

#include <iosfwd>
#include <sstream>
#include <iostream>

class Logger {
public:
	enum Level {
		Fatal = 0, Error, Warn, Info, Debug
	};

	static Logger *GetInstance(void);
	~Logger();

	void initialize(const std::string &logFile);

	/**
	 * Sets the log file.
	 *
	 * This method will open the log file for writing, the former file
	 * contents are removed.
	 *
	 * @param logFile the log file name (may include path).
	 * @param append whether the file is cleaned up before logging in.
	 *
	 * @exception std::ios::failure if the log file could not be opened.
	 */
	void setLogFile(const std::string &logFile, bool append = false);

	/**
	 * Add/removes the timestamp.
	 *
	 * @param flag if true, a log message will always be timestamped
	 *             (default = true).
	 */
	void setTimestamp(bool flag = true) {
		mHasTimestamp = flag;
	}

	/**
	 * Sets tee mode.
	 *
	 * @param flag if true, write messages to both the standard (or error)
	 *        output and the log file (if set) (default = true).
	 */
	void setTeeMode(bool flag = true) {
		mTeeMode = flag;
	}

	/**
	 * Sets the verbosity level of the logger.
	 *
	 * @param verbosity is the level of verbosity.
	 */
	void setVerbosity(Level verbosity) {
		mVerbosity = verbosity;
	}

	/**
	 * Enable logrotation based on the maximum filesize given in
	 * setMaxLogfileSize.
	 *
	 * @param enable Set to true to enable logrotation.
	 */
	void setLogRotation(bool enable) {
		mLogRotation = enable;
	}

	/**
	 * Sets the maximum size of a logfile before logrotation occurs.
	 *
	 * @param kiloBytes Maximum size of logfile in bytes. Defaults to 1MB.
	 */
	void setMaxLogfileSize(long kiloBytes = 1024) {
		mMaxFileSize = kiloBytes;
	}

	/**
	 * Sets whether the logfile switches when changing date.
	 *
	 * @param switchLogEachDay Keeps whether the parameter is activated.
	 */
	void setSwitchLogEachDay(bool switchLogEachDay) {
		mSwitchLogEachDay = switchLogEachDay;
	}

	static void LOG(Level verbosity, const char *format, ...);

	Level mVerbosity; /**< Verbosity level. */

	std::string mLastCallDate;

	std::string mOldDate;
private:
	Logger();

	static Logger *m_instance;

	int m_filefd;
	bool mHasTimestamp; /**< Timestamp flag. */
	bool mTeeMode; /**< Tee mode flag. */

	std::string mFilename; /**< Name of the current logfile. */

	/** Enable rotation of logfiles by size. */
	bool mLogRotation;
	/** Maximum size of current logfile in bytes */
	long mMaxFileSize;
	/** Sets whether the logfile switches when changing date. */
	bool mSwitchLogEachDay;

	int m_file_size;

	void output(Level verbosity, const char *buf);

	void outToFile(Level verbosity, const char *buf);

	void writeLog(const char *format, ...);

	/**
	 * Switch the log file based on a maximum size
	 * and/or and a date change.
	 */
	void switchLogs();
};


// HELPER MACROS
#ifndef __LOG__DEBUG

#define LOG_DEBUG(format, ...)  Logger::LOG(Logger::Debug, "File: " __FILE__ ", Line: %03d: " format "\n", __LINE__, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  Logger::LOG(Logger::Info,"File: " __FILE__ ", Line: %03d: " format "\n", __LINE__, ##__VA_ARGS__)
#define LOG_WARN(format, ...)   Logger::LOG(Logger::Warn, "File: " __FILE__ ", Line: %03d: " format "\n", __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)  Logger::LOG(Logger::Error, "File: " __FILE__ ", Line: %03d: " format "\n", __LINE__, ##__VA_ARGS__)
#define LOG_FATAL(format, ...)  Logger::LOG(Logger::Fatal, "File: " __FILE__ ", Line: %03d: " format "\n", __LINE__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#define LOG_INFO(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERROR(format, ...)
#define LOG_FATAL(format, ...)
#endif

#endif // LOGGER_H
