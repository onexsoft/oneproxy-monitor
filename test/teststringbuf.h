/*
 * teststringbuf.h
 *
 *  Created on: 2016年8月15日
 *      Author: hui
 */

#ifndef TEST_TESTSTRINGBUF_H_
#define TEST_TESTSTRINGBUF_H_

#include "unittest/cpptest.h"
class TestStringBuf:public Test::Suite{

public:
	TestStringBuf() {
		TEST_ADD(TestStringBuf::log_appendStr);
		TEST_ADD(TestStringBuf::log_appendData);
		TEST_ADD(TestStringBuf::log_insert);
		TEST_ADD(TestStringBuf::log_appendFormat);
		TEST_ADD(TestStringBuf::log_erase);
	}
private:
	void log_appendStr();
	void log_appendData();
	void log_insert();
	void log_appendFormat();
	void log_erase();
};

#endif /* TEST_TESTSTRINGBUF_H_ */
