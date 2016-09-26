/*
 * testlogger.h
 *
 *  Created on: 2016年8月1日
 *      Author: hui
 */

#ifndef TEST_TESTLOGGER_H_
#define TEST_TESTLOGGER_H_

#include "unittest/cpptest.h"
class TestLogger:public Test::Suite{

public:
	TestLogger() {
		TEST_ADD(TestLogger::log_test);
	}
private:
	void log_test();
};

#endif /* TEST_TESTLOGGER_H_ */
