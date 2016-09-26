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

int ProtocolPacket::get_byteVarChar(StringBuf& stringBuf, std::string& desStr)
{
	u_uint8 tlen = 0;
	quick_parse_integer_LT(8, stringBuf, tlen, "get byteVarChar length error");
	tlen = tlen * 2;

	uif (cmpdata(unsigned int, tlen, >, stringBuf.get_remailLength())) {
		logs(Logger::ERR, "tlen(%d) is too big, stringBuf length: %d, offset: %d",
				tlen, stringBuf.length(), stringBuf.get_offset());
		return -1;
	}

	u_uint8* tbuf = (u_uint8*)MemManager::malloc(tlen);
	uif(tbuf == NULL) {
		logs(Logger::ERR, "MemManager malloc(%d) error", tlen);
		return -1;
	}

	int ret = 0;
	do {
		uif(ProtocolPacket::get_dataByLen(stringBuf, tbuf, tlen)) {
			logs(Logger::ERR, "parse byte varchar error");
			ret = -1;
			break;
		}

		uif(Tool::unicode2string(tbuf, tlen, desStr)) {
			logs(Logger::ERR, "unicode2string error");
			ret = -1;
			break;
		}
	} while(0);
	MemManager::free(tbuf);

	return ret;
}

int ProtocolPacket::get_uVarChar(StringBuf& stringBuf, std::string& desStr)
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

	//申请空间并且进行是否申请成功，进行正确性校验
	u_uint8* tbuf = (u_uint8*)MemManager::malloc(tlen);
	uif (tbuf == NULL) {
		logs(Logger::ERR, "memManager malloc(%d)", (int)tlen);
		return -1;
	}

	int ret = 0;
	do{
		uif (ProtocolPacket::get_dataByLen(stringBuf, tbuf, tlen)) {
			logs(Logger::ERR, "parse byte varchar error");
			ret = -1;
			break;
		}

		uif(Tool::unicode2string(tbuf, tlen, desStr)) {
			logs(Logger::ERR, "unicode2string error");
			ret = -1;
			break;
		}
	}while(0);
	MemManager::free(tbuf);

	return ret;
}

int ProtocolPacket::get_usVarChar(StringBuf& stringBuf, std::string& desStr)
{
	u_uint16 tlen = 0;

	quick_parse_integer_LT(16, stringBuf, tlen, "get uVarChar length error");
	tlen = tlen * 2;
	uif (cmpdata(unsigned int, tlen, >, stringBuf.length() - stringBuf.get_offset())) {
		logs(Logger::ERR, "tlen(%d) > stringBuf length(%d), stringBuf offset: %d",
				(int)tlen, (int)stringBuf.length(), stringBuf.get_offset());
		return -1;
	}

	u_uint8* tbuf = (u_uint8*)MemManager::malloc(tlen);
	uif (tbuf == NULL) {
		logs(Logger::ERR, "memManager::malloc(%d) error", (int)tlen);
		return -1;
	}

	int ret = 0;
	do {
		uif (ProtocolPacket::get_dataByLen(stringBuf, tbuf, tlen)) {
			logs(Logger::ERR, "parse byte varchar error");
			ret = -1;
			break;
		}

		uif(Tool::ucs22string(tbuf, tlen, desStr)) {
			logs(Logger::ERR, "unicode2string error");
			ret = -1;
			break;
		}
	} while(0);

	MemManager::free(tbuf);
	return ret;
}

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

