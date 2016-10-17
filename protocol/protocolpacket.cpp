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
* @FileName: protocolpacket.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月19日
*
*/

#include <string.h>
#include <stdio.h>

#include "logger.h"
#include "tool.h"
#include "protocolpacket.h"
#include "memmanager.h"

const int __const_litte_end = 0x12345678;
#define is_bigEnd() (((char*)(&__const_litte_end))[0] == 0x12)

ProtocolPacket::ProtocolPacket() {
	// TODO Auto-generated constructor stub

}

ProtocolPacket::~ProtocolPacket() {
	// TODO Auto-generated destructor stub
}

int ProtocolPacket::get_dataByLen(StringBuf& stringBuf, u_uint8* buf, unsigned int len)
{
	uif (stringBuf.length() - stringBuf.get_offset() < len) {
		logs(Logger::ERR, "stringbuf length(%d) - offset(%d) < len(%d)", stringBuf.length(), stringBuf.get_offset(), len);
		return -1;
	}

	uif (buf == NULL)
		return -1;

	memmove((void*)buf, (void*)(stringBuf.addr() + stringBuf.get_offset()), len);
	stringBuf.set_offset(stringBuf.get_offset() + len);
	return 0;
}

int ProtocolPacket::get_integerDataByLen(StringBuf& stringBuf, unsigned int size, u_uint64& value)
{
	value = 0;
	uif (stringBuf.length() - stringBuf.get_offset() < size)
		return -1;

	{
		unsigned int i = 0;
		if (is_bigEnd()) {
			forrange(i, 0, size) {
				value |= (0xff & *(stringBuf.addr() + stringBuf.get_offset() + i)) << (i * 8);
			}
		} else {
			forrange(i, 0, size) {
				value |= (0xff & *(stringBuf.addr() + stringBuf.get_offset() + i)) << (size - i - 1) * 8;
			}
		}
		stringBuf.set_offset(stringBuf.get_offset() + size);
	}

	return 0;
}

int ProtocolPacket::get_integerData8(StringBuf& stringBuf, u_uint64& value)
{
	return ProtocolPacket::get_integerDataByLen(stringBuf, 1, value);
}

int ProtocolPacket::get_integerData16(StringBuf& stringBuf, u_uint64& value)
{
	return ProtocolPacket::get_integerDataByLen(stringBuf, 2, value);
}

int ProtocolPacket::get_integerData32(StringBuf& stringBuf, u_uint64& value)
{
	return ProtocolPacket::get_integerDataByLen(stringBuf, 4, value);
}

int ProtocolPacket::get_integerData64(StringBuf& stringBuf, u_uint64& value)
{
	return ProtocolPacket::get_integerDataByLen(stringBuf, 8, value);
}


int ProtocolPacket::get_integerDataByLen_LT(StringBuf& stringBuf, unsigned int size, u_uint64& value)
{
	value = 0;
	uif (stringBuf.length() - stringBuf.get_offset() < size) {
		logs(Logger::ERR, "length(%d) - offset(%d) = size(%d)", stringBuf.length(), stringBuf.get_offset(), size);
		return -1;
	}

	{
		unsigned int i = 0;
		if (is_bigEnd()) {
			forrange(i, 0, size) {
				value |= (0xff & *((char*)(stringBuf.addr() + stringBuf.get_offset() + i))) << (size - i - 1) * 8;
			}
		} else {
			forrange(i, 0, size) {
				value |= (0xff & *((char*)(stringBuf.addr() + stringBuf.get_offset() + i))) << (i * 8);
			}
		}
		stringBuf.set_offset(stringBuf.get_offset() + size);
	}

	return 0;
}

int ProtocolPacket::get_integerData8_LT(StringBuf& stringBuf, u_uint64& value)
{
	return ProtocolPacket::get_integerDataByLen_LT(stringBuf, 1, value);
}

int ProtocolPacket::get_integerData16_LT(StringBuf& stringBuf, u_uint64& value)
{
	return ProtocolPacket::get_integerDataByLen_LT(stringBuf, 2, value);
}

int ProtocolPacket::get_integerData32_LT(StringBuf& stringBuf, u_uint64& value)
{
	return ProtocolPacket::get_integerDataByLen_LT(stringBuf, 4, value);
}

int ProtocolPacket::get_integerData64_LT(StringBuf& stringBuf, u_uint64& value)
{
	return ProtocolPacket::get_integerDataByLen_LT(stringBuf, 8, value);
}

int ProtocolPacket::get_unicodeString1BLen(StringBuf& stringBuf, std::string& desStr)
{
	u_uint8 tlen = 0;
	quick_parse_integer_LT(8, stringBuf, tlen, "get byteVarChar length error");
	tlen = tlen * 2;
	uif(tlen <= 0)
		return 0;

	uif (cmpdata(unsigned int, tlen, >, stringBuf.get_remailLength())) {
		logs(Logger::ERR, "tlen(%d) is too big, stringBuf length: %d, offset: %d",
				tlen, stringBuf.length(), stringBuf.get_offset());
		return -1;
	}

	StringBuf tbuf;
	uif(tbuf.reallocMem(tlen)) {
		logs(Logger::ERR, "reallocMem error(len: %d)", tlen);
		return -1;
	}


	uif(ProtocolPacket::get_dataByLen(stringBuf, (u_uint8*)tbuf.addr(), tlen)) {
		logs(Logger::ERR, "parse byte varchar error");
		return -1;
	}

	uif(Tool::byte2string((const u_uint8*)tbuf.addr(), tlen, desStr)) {
		logs(Logger::ERR, "unicode2string error");
		return -1;
	}

	return 0;
}

int ProtocolPacket::get_unicodeString2BLen(StringBuf& stringBuf, std::string& desStr)
{
	u_uint16 tlen = 0;

	//读取需要读取的数据长度，并且进行正确性校验
	quick_parse_integer_LT(16, stringBuf, tlen, "get uVarChar length error");
	tlen = tlen * 2;
	uif (cmpdata(unsigned int, tlen, >, stringBuf.length() - stringBuf.get_offset())) {
		logs(Logger::ERR, "tlen(%d) is too big, stringBuf length: %d, offset: %d",
				tlen, stringBuf.length(), stringBuf.get_offset());
		return -1;
	}

	StringBuf tbuf;
	uif(tbuf.reallocMem(tlen)) {
		logs(Logger::ERR, "reallocMem error(len: %d)", tlen);
		return -1;
	}

	uif (ProtocolPacket::get_dataByLen(stringBuf, (u_uint8*)tbuf.addr(), tlen)) {
		logs(Logger::ERR, "parse byte varchar error");
		return -1;
	}

	uif(Tool::byte2string((const u_uint8*)tbuf.addr(), tlen, desStr)) {
		logs(Logger::ERR, "unicode2string error");
		return -1;
	}
	return 0;
}

int ProtocolPacket::get_unicodeWString2BLen(StringBuf& stringBuf, std::wstring& desWStr)
{
	u_uint16 tlen = 0;

	quick_parse_integer_LT(16, stringBuf, tlen, "get uVarChar length error");
	tlen = tlen * 2;
	uif (cmpdata(unsigned int, tlen, >, stringBuf.length() - stringBuf.get_offset())) {
		logs(Logger::ERR, "tlen(%d) > stringBuf length(%d), stringBuf offset: %d",
				(int)tlen, (int)stringBuf.length(), stringBuf.get_offset());
		return -1;
	}

	StringBuf tbuf;
	uif(tbuf.reallocMem(tlen)) {
		logs(Logger::ERR, "reallocMem error(len: %d)", tlen);
		return -1;
	}

	uif (ProtocolPacket::get_dataByLen(stringBuf, (u_uint8*)tbuf.addr(), tlen)) {
		logs(Logger::ERR, "parse byte varchar error");
		return -1;
	}

	uif(Tool::byte2wstring((const u_uint8*)tbuf.addr(), tlen, desWStr)) {
		logs(Logger::ERR, "unicode2string error");
		return -1;
	}
	return 0;
}

int ProtocolPacket::get_unicodeStringByLen(StringBuf& stringBuf, const unsigned int len, std::string& desStr)
{
	StringBuf tbuf;
	uif(tbuf.reallocMem(len)) {
		logs(Logger::ERR, "reallocMem error(len: %d)", len);
		return -1;
	}

	uif (ProtocolPacket::get_dataByLen(stringBuf, (u_uint8*)tbuf.addr(), len)) {
		logs(Logger::ERR, "get databylen error");
		return -1;
	}

	uif(Tool::byte2string((const u_uint8*)tbuf.addr(), len, desStr)) {
		logs(Logger::ERR, "unicode2string error");
		return -1;
	}
	return 0;
}

int ProtocolPacket::get_stringBuf1BLen(StringBuf& stringBuf, StringBuf& desStr)
{
	u_uint8 dataLen = 0;
	quick_parse_integer_LT(8, stringBuf, dataLen, "parse param name length error");

	if (dataLen <= 0)
		return 0;

	desStr.mallocMem(dataLen);

	quick_parse_data(stringBuf, (u_uint8*)desStr.addr(), dataLen, "read data error");

	return 0;
}

//int ProtocolPacket::get_byteVarChar(StringBuf& stringBuf, std::string& desStr)
//{
//	u_uint8 tlen = 0;
//	quick_parse_integer_LT(8, stringBuf, tlen, "get byteVarChar length error");
//	tlen = tlen * 2;
//
//	uif (cmpdata(unsigned int, tlen, >, stringBuf.get_remailLength())) {
//		logs(Logger::ERR, "tlen(%d) is too big, stringBuf length: %d, offset: %d",
//				tlen, stringBuf.length(), stringBuf.get_offset());
//		return -1;
//	}
//
//	u_uint8* tbuf = (u_uint8*)MemManager::malloc(tlen);
//	uif(tbuf == NULL) {
//		logs(Logger::ERR, "MemManager malloc(%d) error", tlen);
//		return -1;
//	}
//
//	int ret = 0;
//	do {
//		uif(ProtocolPacket::get_dataByLen(stringBuf, tbuf, tlen)) {
//			logs(Logger::ERR, "parse byte varchar error");
//			ret = -1;
//			break;
//		}
//
//		uif(Tool::unicode2string(tbuf, tlen, desStr)) {
//			logs(Logger::ERR, "unicode2string error");
//			ret = -1;
//			break;
//		}
//	} while(0);
//	MemManager::free(tbuf);
//
//	return ret;
//}
//
//int ProtocolPacket::get_uVarChar(StringBuf& stringBuf, std::string& desStr)
//{
//	u_uint16 tlen = 0;
//
//	//读取需要读取的数据长度，并且进行正确性校验
//	quick_parse_integer_LT(16, stringBuf, tlen, "get uVarChar length error");
//	tlen = tlen * 2;
//	uif (cmpdata(unsigned int, tlen, >, stringBuf.length() - stringBuf.get_offset())) {
//		logs(Logger::ERR, "tlen(%d) is too big, stringBuf length: %d, offset: %d",
//				tlen, stringBuf.length(), stringBuf.get_offset());
//		return -1;
//	}
//
//	//申请空间并且进行是否申请成功，进行正确性校验
//	u_uint8* tbuf = (u_uint8*)MemManager::malloc(tlen);
//	uif (tbuf == NULL) {
//		logs(Logger::ERR, "memManager malloc(%d)", (int)tlen);
//		return -1;
//	}
//
//	int ret = 0;
//	do{
//		uif (ProtocolPacket::get_dataByLen(stringBuf, tbuf, tlen)) {
//			logs(Logger::ERR, "parse byte varchar error");
//			ret = -1;
//			break;
//		}
//
//		uif(Tool::unicode2string(tbuf, tlen, desStr)) {
//			logs(Logger::ERR, "unicode2string error");
//			ret = -1;
//			break;
//		}
//	}while(0);
//	MemManager::free(tbuf);
//
//	return ret;
//}
//
//int ProtocolPacket::get_usVarChar(StringBuf& stringBuf, std::string& desStr)
//{
//	u_uint16 tlen = 0;
//
//	quick_parse_integer_LT(16, stringBuf, tlen, "get uVarChar length error");
//	tlen = tlen * 2;
//	uif (cmpdata(unsigned int, tlen, >, stringBuf.length() - stringBuf.get_offset())) {
//		logs(Logger::ERR, "tlen(%d) > stringBuf length(%d), stringBuf offset: %d",
//				(int)tlen, (int)stringBuf.length(), stringBuf.get_offset());
//		return -1;
//	}
//
//	u_uint8* tbuf = (u_uint8*)MemManager::malloc(tlen);
//	uif (tbuf == NULL) {
//		logs(Logger::ERR, "memManager::malloc(%d) error", (int)tlen);
//		return -1;
//	}
//
//	int ret = 0;
//	do {
//		uif (ProtocolPacket::get_dataByLen(stringBuf, tbuf, tlen)) {
//			logs(Logger::ERR, "parse byte varchar error");
//			ret = -1;
//			break;
//		}
//
//		uif(Tool::ucs22string(tbuf, tlen, desStr)) {
//			logs(Logger::ERR, "unicode2string error");
//			ret = -1;
//			break;
//		}
//	} while(0);
//
//	MemManager::free(tbuf);
//	return ret;
//}

u_uint64 ProtocolPacket::buffer2Integer(const void* buffer, const unsigned int len, bool is_littleEndian)
{
	uif (len > 8)
		return -1;

	u_uint64 value = 0;
	{

		unsigned int i = 0;
		unsigned int size = len;
		if (is_littleEndian == false) {
			forrange(i, 0, size) {
				value |= (0xff & *(((char*)buffer) + i)) << (size - i - 1) * 8;
			}
		} else {
			forrange(i, 0, size) {
				value |= (0xff & *(((char*)buffer) + i)) << (i * 8);
			}
		}
	}
	return value;
}

int ProtocolPacket::get_string(StringBuf& stringBuf, std::string& desStr)
{
	char *p = (char*)memchr((stringBuf.addr() + stringBuf.get_offset()), 0, stringBuf.get_remailLength());
	unsigned int len = (p - stringBuf.addr()) - stringBuf.get_offset() + 1;
	uif (len <= 0)
		return -1;

	desStr.append(std::string((char*)(stringBuf.addr() + stringBuf.get_offset()), len - 1));
	stringBuf.set_offset(stringBuf.get_offset() + len);

	return 0;
}

int ProtocolPacket::get_byteDataByPosLen(StringBuf& stringBuf, StringBuf& desStr,
			const unsigned int pos, const unsigned int len)
{
	if (stringBuf.length() < pos + len)
			return -1;
	desStr.reallocMem(desStr.get_length() + len);
	memcpy((void*)(desStr.addr() + desStr.get_length()), (void*)(stringBuf.addr() + pos), len);

	desStr.set_length(desStr.get_length() + len);
	return 0;
}

int ProtocolPacket::get_unicodeStringByPosLen(StringBuf& stringBuf, std::string& desStr,
			const unsigned int pos, const unsigned int len)
{
	StringBuf tbuf;
	uif (ProtocolPacket::get_byteDataByPosLen(stringBuf, tbuf, pos, len)) {
		logs(Logger::ERR, "get unicode error, pos: %d, len: %d", pos, len);
		return -1;
	}
	uif(Tool::byte2string((const unsigned char*)tbuf.addr(), tbuf.get_length(), desStr)) {
		logs(Logger::ERR, "unicode to string error");
		return -1;
	}
	return 0;
}

int ProtocolPacket::get_unicodeWStringByPosLen(StringBuf& stringBuf, std::wstring& desStr,
			const unsigned int pos, const unsigned int len)
{
	StringBuf tbuf;
	uif (ProtocolPacket::get_byteDataByPosLen(stringBuf, tbuf, pos, len)) {
		logs(Logger::ERR, "get unicode error, pos: %d, len: %d", pos, len);
		return -1;
	}
	uif(Tool::byte2wstring((const unsigned char*)tbuf.addr(), tbuf.get_length(), desStr)) {
		logs(Logger::ERR, "unicode to string error");
		return -1;
	}
	return 0;
}

//
//int ProtocolPacket::get_unicode(StringBuf& stringBuf, StringBuf& desStr,
//		const unsigned int pos, const unsigned int len)
//{
//	if (stringBuf.length() < pos + len)
//			return -1;
//	desStr.reallocMem(desStr.get_length() + len);
//	memcpy((void*)(desStr.addr() + desStr.get_length()), (void*)(stringBuf.addr() + pos), len);
//
//	desStr.set_length(desStr.get_length() + len);
//	return 0;
//}
//
//int ProtocolPacket::get_ucString(StringBuf& stringBuf, std::string& desStr,
//		const unsigned int pos, const unsigned int len)
//{
//	StringBuf tbuf;
//	uif (ProtocolPacket::get_unicode(stringBuf, tbuf, pos, len)) {
//		logs(Logger::ERR, "get unicode error, pos: %d, len: %d", pos, len);
//		return -1;
//	}
//	uif(Tool::unicode2string((const unsigned char*)tbuf.addr(), tbuf.get_length(), desStr)) {
//		logs(Logger::ERR, "unicode to string error");
//		return -1;
//	}
//	return 0;
//}

int ProtocolPacket::set_dataByLen(StringBuf& stringBuf, unsigned char* buf, unsigned int len)
{
	stringBuf.reallocMem(stringBuf.get_length() + len);
	memmove((unsigned char*)(stringBuf.addr() + stringBuf.get_length()), buf, len);
	stringBuf.set_length(stringBuf.get_length() + len);
	return 0;
}

int ProtocolPacket::set_integerDataByLen(StringBuf& stringBuf, unsigned int size, u_uint64 value)
{
	stringBuf.reallocMem(stringBuf.get_length() + size);
	if (set_integerDataByLen(stringBuf, size, value, stringBuf.get_length())) {
		logs(Logger::ERR, "set integer error, size: %d, length: %d, allocate_len: %d",
				size, stringBuf.get_length(), stringBuf.get_allocateLen());
		return -1;
	}
	stringBuf.set_length(stringBuf.get_length() + size);

	return 0;
}

int ProtocolPacket::set_integerDataByLen(StringBuf& stringBuf, unsigned int size, u_uint64 value, int position)
{
	if (position + size > stringBuf.get_allocateLen()) {
		return -1;
	}
	char* tbuf = (char*)(stringBuf.addr() + position);

	unsigned int i = 0;
	if (is_bigEnd()) {
		forrange(i, 0, size) {
			tbuf[i] = 0xff & (value >> (i * 8));
		}
	} else {
		forrange(i, 0, size) {
			tbuf[i] = 0xff & (value >> ((size - i - 1) * 8));
		}
	}
	return 0;
}

int ProtocolPacket::set_integerData8(StringBuf& stringBuf, u_uint64 value)
{
	return set_integerDataByLen(stringBuf, 1, value);
}

int ProtocolPacket::set_integerData16(StringBuf& stringBuf, u_uint64 value)
{
	return set_integerDataByLen(stringBuf, 2, value);
}

int ProtocolPacket::set_integerData32(StringBuf& stringBuf, u_uint64 value)
{
	return set_integerDataByLen(stringBuf, 4, value);
}

int ProtocolPacket::set_integerData64(StringBuf& stringBuf, u_uint64 value)
{
	return set_integerDataByLen(stringBuf, 8, value);
}

int ProtocolPacket::set_integerDataByLen_LT(StringBuf& stringBuf, unsigned int size, u_uint64 value)
{
	stringBuf.reallocMem(stringBuf.get_length() + size);

	if (set_integerDataByLen_LT(stringBuf, size, value, stringBuf.get_length())) {
		logs(Logger::ERR, "set integer error, size: %d, length: %d, allocate_len: %d",
				size, stringBuf.get_length(), stringBuf.get_allocateLen());
		return -1;
	}
	stringBuf.set_length(stringBuf.get_length() + size);

	return 0;
}

int ProtocolPacket::set_integerDataByLen_LT(StringBuf& stringBuf, unsigned int size, u_uint64 value, int position)
{
	if (position + size >= stringBuf.get_allocateLen()) {
		return -1;
	}

	char* tbuf = (char*)(stringBuf.get_buf() + position);

	unsigned int i = 0;
	if (is_bigEnd()) {
		forrange(i, 0, size) {
			tbuf[i] = 0xff & (value >> ((size - i - 1) * 8));
		}
	} else {
		forrange(i, 0, size) {
			tbuf[i] = 0xff & (value >> (i * 8));
		}
	}
	return 0;
}

int ProtocolPacket::set_integerData8_LT(StringBuf& stringBuf, u_uint64 value)
{
	return set_integerDataByLen_LT(stringBuf, 1, value);
}

int ProtocolPacket::set_integerData16_LT(StringBuf& stringBuf, u_uint64 value)
{
	return set_integerDataByLen_LT(stringBuf, 2, value);
}

int ProtocolPacket::set_integerData32_LT(StringBuf& stringBuf, u_uint64 value)
{
	return set_integerDataByLen_LT(stringBuf, 4, value);
}

int ProtocolPacket::set_integerData64_LT(StringBuf& stringBuf, u_uint64 value)
{
	return set_integerDataByLen_LT(stringBuf, 8, value);
}

int ProtocolPacket::set_stringUnicode1BLen(StringBuf& stringBuf, const std::string& desStr)
{
	int length = desStr.length();

	StringBuf tbuf;
	uif(Tool::string2byte(desStr, tbuf)) {
		logs(Logger::ERR, "string2unicode error");
		return -1;
	}

	//1. write length
	quick_set_integer_LT(8, stringBuf, length, "set length error");

	//2. write data
	stringBuf.append((void*)(tbuf.addr() + tbuf.get_offset()), tbuf.get_remailLength());

	return 0;
}

int ProtocolPacket::set_stringUnicode2BLen(StringBuf& stringBuf, const std::string& desStr)
{
	int length = desStr.length();

	StringBuf tbuf;
	uif(Tool::string2byte(desStr, tbuf)) {
		logs(Logger::ERR, "string2unicode error");
		return -1;
	}

	//1. write length
	quick_set_integer_LT(16, stringBuf, length, "set length error");

	//2. write data
	stringBuf.append((void*)(tbuf.addr() + tbuf.get_offset()), tbuf.get_remailLength());

	return 0;
}

int ProtocolPacket::set_wstringUnicode2BLen(StringBuf& stringBuf, const std::wstring& desStr)
{
	int length = desStr.length();

	StringBuf tbuf;
	uif(Tool::wstring2byte(desStr, tbuf)) {
		logs(Logger::ERR, "string2unicode error");
		return -1;
	}

	//1. write length
	quick_set_integer_LT(16, stringBuf, length, "set length error");

	//2. write data
	stringBuf.append((void*)(tbuf.addr() + tbuf.get_offset()), tbuf.get_remailLength());

	return 0;
}



//int ProtocolPacket::set_byteVarChar(StringBuf& stringBuf, const std::string& desStr)
//{
//	int length = desStr.length();
//
//	StringBuf tbuf;
//	uif(Tool::string2unicode(desStr, tbuf)) {
//		logs(Logger::ERR, "string2unicode error");
//		return -1;
//	}
//
//	//1. write length
//	quick_set_integer_LT(8, stringBuf, length, "set length error");
//
//	//2. write data
//	stringBuf.append((void*)(tbuf.addr() + tbuf.get_offset()), tbuf.get_remailLength());
//
//	return 0;
//}
//
//int ProtocolPacket::set_byteVarChar(StringBuf& desBuf, StringBuf& srcBuf)
//{
//	int length = srcBuf.get_remailLength();
//	StringBuf tbuf;
//	uif (Tool::string2unicode(srcBuf, tbuf)) {
//		logs(Logger::ERR, "string2unicode error");
//		return -1;
//	}
//
//	//1. write length
//	quick_set_integer_LT(8, desBuf, length, "set length error");
//
//	//2. write data
//	desBuf.append((void*)(tbuf.addr() + tbuf.get_offset()), tbuf.get_remailLength());
//
//	return 0;
//}
