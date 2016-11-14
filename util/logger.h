/*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
* @FileName: logger.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月29日
*
*/

#ifndef UTIL_LOGGER_H_
#define UTIL_LOGGER_H_

#include "define.h"
#ifdef WIN32
#include "windows.h"
#else
#include <pthread.h>
#endif
#include <string>
#include <iostream>
#include <stdarg.h>
#include <stdlib.h>

#define logs(level, fmt, args...) do{\
	if (Logger::get_logger()->get_logLevel() <= level) \
		Logger::get_logger()->log(level, "[%s %s %d]"fmt, __FILE__, __func__, __LINE__, ##args);\
}while(0)

#define logs_buf(name, buf, bufLen) do{\
	if (Logger::get_logger()->get_dumpData()) \
		Logger::get_logger()->log_hex((char*)name, buf, bufLen); \
}while(0)
#define logs_buf_force(name, buf, bufLen) do{\
	if (Logger::get_logger()->get_logLevel() <= Logger::DEBUG) \
		Logger::get_logger()->log_hex((char*)name, buf, bufLen);\
}while(0)
#define logs_logsql(fmt, args...) do{\
		if (Logger::get_logger()->get_logSql()) \
			Logger::get_logger()->log_force(fmt, ##args);\
}while(0)

#define logs_setLevel(level) Logger::get_logger()->set_logLevel(level)
#define logs_setBatchNum(batchNum) Logger::get_logger()->set_batchNum(batchNum)
#define logs_finished() Logger::get_logger()->flush()
#define logs_setDumpData(dump) Logger::get_logger()->set_dumpData(dump)
#define logs_setLogFile(filePath) Logger::get_logger()->set_logFilePath(filePath)
#define logs_setLogSql(logsql) Logger::get_logger()->set_logSql(logsql)

class Logger{
public:
	typedef enum{
		DEBUG,
		INFO,
		WARNING,
		ERR, //error
		FATAL,
		LEVEL_SUM
	}loggerLevel;

public:
	static Logger* get_logger();
	void log_force(const char* fmt, ...);
	void log(loggerLevel level, const char* fmt, ...);
	void flush();
	void log_hex(char* name, void *data, int dataLen);


private:
	Logger(){
		this->set_logLevel(Logger::INFO);
		this->set_batchNum(1);
		this->set_currentNum(0);
		this->set_dumpData(false);
		this->set_logSql(false);
		this->set_logFileFd(NULL);
	};
	Logger(const Logger&);
	~Logger();
	Logger& operator= (const Logger& );
	void output(const char* hint, const char* fmt, va_list args);
	void outputStrategy(const char* logstr);
	void outputConsoleOrFile(const char* logstr);
	std::string current_timeStr();

private:
#ifdef WIN32
	static HANDLE mutex;
#else
	static pthread_mutex_t mutex;
#endif

	declare_class_member(loggerLevel, logLevel)
	declare_class_member(int, batchNum)
	declare_class_member(int, currentNum)
	declare_class_member(std::string, logList)
	declare_class_member(bool, dumpData)
	declare_class_member(bool, logSql)
	declare_class_member(std::string, logFilePath);
	declare_class_member(FILE*, logFileFd);
};

#endif /* UTIL_LOGGER_H_ */
