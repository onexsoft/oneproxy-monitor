/*
 * testlogger.cpp
 *
 *  Created on: 2016年8月1日
 *      Author: hui
 */

#include "testlogger.h"

#include "../util/logger.h"

void TestLogger::log_test() {
	logs(Logger::INFO, "test INFO log function");
	logs(Logger::DEBUG, "test DEBUG log function");
	logs(Logger::ERR, "test ERR log function");
	logs(Logger::ERR, "TEST TEST :%d", 12345);
	logs(Logger::ERR, "TEST TEST :%s", "xxxxxxxxxxx");
	logs(Logger::ERR, "TEST TEST (%s)", "xxxxxxxxxxx");
}


