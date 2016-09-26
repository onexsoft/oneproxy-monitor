/*
 * teststringbuf.cpp
 *
 *  Created on: 2016年8月15日
 *      Author: hui
 */

#include "teststringbuf.h"
#include <string.h>
#include "../util/logger.h"
#include "../util/stringbuf.h"

void TestStringBuf::log_appendData()
{
	StringBuf sb;
	char *a = (char*)"hello ";
	sb.append(a, strlen(a));

	char *b = (char*)"world";
	sb.append(b, strlen(b));

	TEST_ASSERT(!strcmp(sb.addr(), "hello world"));
}

void TestStringBuf::log_appendStr()
{
	StringBuf sb;
	char *a = (char*)"hello ";
	sb.append(a);

	char *b = (char*)"world";
	sb.append(b);

	TEST_ASSERT(!strcmp(sb.addr(), "hello world"));
}

void TestStringBuf::log_insert()
{
	StringBuf sb;
	char *a = (char*)"world";
	sb.insert(0, (void*)a, strlen(a));

	char *b = (char*)"hello ";
	sb.insert(0, (void*)b, strlen(b));

	TEST_ASSERT(!strcmp(sb.addr(), "hello world"));

	//pos > 0 && pos < len;
	StringBuf sb1;
	char* aaa = (char*)"aaa";
	sb1.insert(0, (void*)aaa, strlen(aaa));
	char* bb = (char*)"bb";
	sb1.insert(1, (void*)bb, strlen(bb));
	char* cc = (char*)"cc";
	sb1.insert(sb1.length(), (void*)cc, strlen(cc));
	TEST_ASSERT(!strcmp(sb1.addr(), "abbaacc"));
}

void TestStringBuf::log_appendFormat()
{
	StringBuf sb;
	sb.appendFormat("%s %d %s %lld %s", "hello world", 123, "kkkkkk", 1234567, "ssssss");
	logs(Logger::ERR, "sb.buf: %s", sb.addr());
}

void TestStringBuf::log_erase()
{
	StringBuf sb, sb1, sb2;
	sb.append("1234567890");
	sb.erase(3, 5);
	TEST_ASSERT(!strcmp(sb.addr(), "12367890"));

	sb1.append("1234567890");
	sb1.erase(-1, 5);
	TEST_ASSERT(!strcmp(sb1.addr(), "67890"));

	sb2.append("1234567890");
	sb2.erase(-1, 20);
	TEST_ASSERT(!strcmp(sb2.addr(), ""));

	sb2.clear();
	sb2.append("1234567890");
	sb2.erase(5, 20);
	TEST_ASSERT(!strcmp(sb2.addr(), "12345"));

	sb2.clear();
	sb2.append("1234567890");
	sb2.erase(5, 4);
	TEST_ASSERT(!strcmp(sb2.addr(), "1234567890"));
}

