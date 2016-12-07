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
* @FileName: logger.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月29日
*
*/

#include "util/logger.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <string>
using namespace std;

#ifdef WIN32
HANDLE Logger::mutex = ::CreateMutex(NULL, false, NULL);
#else
pthread_mutex_t Logger::mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifdef WIN32
#define logger_mutex_lock() ::WaitForSingleObject(mutex, INFINITE)
#define logger_mutex_unlock() ::ReleaseMutex(mutex)
#else
#define logger_mutex_lock() pthread_mutex_lock(&mutex)
#define logger_mutex_unlock() pthread_mutex_unlock(&mutex)
#endif

#define simple_output(hint) do{\
		va_list args;\
		va_start(args, fmt);\
		output(hint, fmt, args);\
		va_end(args);\
} while(0)

Logger::~Logger() {
	if (this->get_logFileFd()) {
		fclose(this->get_logFileFd());
	}
}

Logger* Logger::get_logger() {

	static Logger *logger = NULL;
	if (logger == NULL) {
		logger_mutex_lock();
		if (logger == NULL) {
			logger = new Logger();
		}
		logger_mutex_unlock();
	}
	return logger;
}

void Logger::log_force(const char* fmt, ...)
{
	simple_output(NULL);
}

void Logger::log(loggerLevel level, const char* fmt, ...) {
	const char* hint = NULL;
	switch(level) {
	case INFO:
		hint = "info";
		break;
	case DEBUG:
		hint = "debug";
		break;
	case WARNING:
		hint = "warning";
		break;
	case ERR:
		hint = "error";
		break;
	case FATAL:
		hint = "fatal";
		break;
	case LEVEL_SUM:
		hint = "dump";
		break;
	default:
		hint = "unkown";
		break;
	}
	simple_output(hint);

	if (level == FATAL) {
		this->flush();
		exit(-1);
	}
}

void Logger::output(const char* hint, const char* fmt, va_list args) {
	unsigned long int tid = this->get_threadId();

	char buf[2048], result[2048];
	vsnprintf(buf, 2048, fmt, args);

	if (hint == NULL) {
#if 1
	snprintf(result, 2048, "[%s][%ld]%s\n", this->current_timeStr().c_str(), (unsigned long int)(tid), buf);
#else
	snprintf(result, 2048, "%s\n", buf);
#endif
	} else {
#if 1
	snprintf(result, 2048, "[%s][%s][%ld]%s\n", this->current_timeStr().c_str(), hint, (unsigned long int)(tid), buf);
#else
	snprintf(result, 2048, "[%s]%s\n", hint, buf);
#endif
	}

	outputStrategy(result);
}

unsigned long int Logger::get_threadId()
{
#ifdef WIN32
	DWORD tid = ::GetCurrentThreadId();
#else
	pthread_t tid = pthread_self();
#endif
	return (unsigned long)tid;
}

void Logger::outputStrategy(const char* logstr) {
	if (this->get_batchNum() <= 1) {
		this->outputConsoleOrFile(logstr);
	} else {
		this->m_logList.append(logstr);
		this->m_currentNum ++;
		if (this->m_currentNum >= this->get_batchNum()) {
			this->outputConsoleOrFile(this->m_logList.c_str());
			this->m_currentNum = 0;
			this->m_logList.clear();
		}
	}
}

void Logger::outputConsoleOrFile(const char* logstr) {
	logger_mutex_lock();
	printf("%s", logstr);
	if (this->m_logFilePath.size() > 0)
	{

		if (this->m_logFileFd == NULL) {
//			char buf[1024] = {0};
//			sprintf(buf, "%lu_%s", this->get_threadId(), this->m_logFilePath.c_str());
//			this->m_logFileFd = fopen(buf, "a+");//a+
			this->m_logFileFd = fopen(this->m_logFilePath.c_str(), "a+");
			if (this->m_logFileFd == NULL) {
				printf("fopen file(%s) error", this->m_logFilePath.c_str());
				logger_mutex_unlock();
				return;
			}
		}
		fprintf(this->m_logFileFd, "%s", logstr);
		fclose(this->m_logFileFd);
		this->m_logFileFd = NULL;
	}
	logger_mutex_unlock();
}

void Logger::flush()
{
	if (this->m_logList.size() > 0) {
		this->outputConsoleOrFile(this->m_logList.c_str());
		this->m_currentNum = 0;
		this->m_logList.clear();
	}
}

void Logger::log_hex(char* name, void *data, int dataLen) {
    const char toHex[256][10] = { " 00", " 01", " 02", " 03", " 04", " 05", " 06", " 07", " 08", " 09", " 0A", " 0B", " 0C", " 0D", " 0E", " 0F", " 10", " 11", " 12", " 13", " 14", " 15", " 16", " 17", " 18", " 19", " 1A", " 1B", " 1C", " 1D", " 1E", " 1F", " 20", " 21", " 22", " 23", " 24", " 25", " 26", " 27", " 28", " 29", " 2A", " 2B", " 2C", " 2D", " 2E", " 2F", " 30", " 31", " 32", " 33", " 34", " 35", " 36", " 37", " 38", " 39", " 3A", " 3B", " 3C", " 3D", " 3E", " 3F", " 40", " 41", " 42", " 43", " 44", " 45", " 46", " 47", " 48", " 49", " 4A", " 4B", " 4C", " 4D", " 4E", " 4F", " 50", " 51", " 52", " 53", " 54", " 55", " 56", " 57", " 58", " 59", " 5A", " 5B", " 5C", " 5D", " 5E", " 5F", " 60", " 61", " 62", " 63", " 64", " 65", " 66", " 67", " 68", " 69", " 6A", " 6B", " 6C", " 6D", " 6E", " 6F", " 70", " 71", " 72", " 73", " 74", " 75", " 76", " 77", " 78", " 79", " 7A", " 7B", " 7C", " 7D", " 7E", " 7F", " 80", " 81", " 82", " 83", " 84", " 85", " 86", " 87", " 88", " 89", " 8A", " 8B", " 8C", " 8D", " 8E", " 8F", " 90", " 91", " 92", " 93", " 94", " 95", " 96", " 97", " 98", " 99", " 9A", " 9B", " 9C", " 9D", " 9E", " 9F", " A0", " A1", " A2", " A3", " A4", " A5", " A6", " A7", " A8", " A9", " AA", " AB", " AC", " AD", " AE", " AF", " B0", " B1", " B2", " B3", " B4", " B5", " B6", " B7", " B8", " B9", " BA", " BB", " BC", " BD", " BE", " BF", " C0", " C1", " C2", " C3", " C4", " C5", " C6", " C7", " C8", " C9", " CA", " CB", " CC", " CD", " CE", " CF", " D0", " D1", " D2", " D3", " D4", " D5", " D6", " D7", " D8", " D9", " DA", " DB", " DC", " DD", " DE", " DF", " E0", " E1", " E2", " E3", " E4", " E5", " E6", " E7", " E8", " E9", " EA", " EB", " EC", " ED", " EE", " EF", " F0", " F1", " F2", " F3", " F4", " F5", " F6", " F7", " F8", " F9", " FA", " FB", " FC", " FD", " FE", " FF" };
    const char toChar[256][2] = { ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", "!", "\"", "#", "$", "%", "&", "\"", "(", ")", "*", "+", ",", "-", ".", "/", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_", "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", ".", "." };
    std::string sb;
    std::string tmpbuf;
    int bytesThisLine = 0;
    int bytesPerLine = 8;
    int i = 0;

//    dataLen = dataLen > 100 ? 100 : dataLen;

    for (i = 0; i < dataLen; ++i) {
        int index = (int)(((unsigned char*)data)[i] & 0xff);
        sb.append(toHex[index]);
        tmpbuf.append(toChar[index]);

        bytesThisLine = bytesThisLine + 1;
        if (bytesThisLine == bytesPerLine) {
            sb.append("     |");
            sb.append(tmpbuf);
            sb.append("|\n");
            tmpbuf.clear();
            bytesThisLine = 0;
        }
    }

    if (bytesThisLine > 0) {
        int space = bytesPerLine - bytesThisLine -1;
        for (i = 0; i < (space + 1); ++i) {
            sb.append("   ");
        }
        sb.append("     |");
        sb.append(tmpbuf);

        for (i = 0; i < (space + 1); ++i) {
            sb.append(" ");
        }
        sb.append("|\n");
    }
    this->log(Logger::LEVEL_SUM, "%s:%d", name, dataLen);
    this->outputConsoleOrFile(sb.c_str());
}

void Logger::delete_logFile()
{
	if (this->m_logFilePath.size()) {
		unlink(this->m_logFilePath.c_str());
	}
}

void Logger::log_unicodeStr(char* name, void* data, int dataLen)
{
	if (dataLen %2) {
		logs(Logger::ERR, "unicode str length error");
		return;
	}

	std::string sb;
	const char toHex[256][10] = {
			"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
			"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
			"20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
			"30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
			"40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
			"50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
			"60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
			"70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
			"80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
			"90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
			"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
			"B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
			"C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
			"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
			"E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
			"F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
			};
	for (int i = 0; i < dataLen;) {
		int index = (int)(((unsigned char*)data)[i] & 0xff);
		int index1 = (int)(((unsigned char*)data)[i+1] & 0xff);
		sb.append("\\u");
		sb.append(toHex[index1]);
		sb.append(toHex[index]);
		i = i + 2;
	}
	sb.append("\n");

	this->log(Logger::LEVEL_SUM, "%s:%d", name, dataLen);
	this->outputConsoleOrFile(sb.c_str());
}

std::string Logger::current_timeStr()
{
	char buf[32];

#ifdef linux
	time_t ttime = time(NULL);
	struct tm tm = {0};
	localtime_r(&ttime, &tm);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tm);
#else
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d", sys.wYear, sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond);
#endif

	return std::string(buf, strlen(buf));
}
