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
* @FileName: protocolpacket.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月19日
*
*/

#ifndef PROTOCOL_PROTOCOLPACKET_H_
#define PROTOCOL_PROTOCOLPACKET_H_

#include "stringbuf.h"
#include "define.h"

#define quick_parse_integer_LT(funcIndex, dataBuf, value, errorMsg) do{\
	u_uint64 tmpvalue = 0; \
	if (ProtocolPacket::get_integerData##funcIndex##_LT(dataBuf, tmpvalue)) { \
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
	value = (u_uint##funcIndex)tmpvalue;\
}while(0)

#define quick_parse_integer(funcIndex, dataBuf, value, errorMsg) do{\
	u_uint64 tmpvalue = 0; \
	if (ProtocolPacket::get_integerData##funcIndex(dataBuf, tmpvalue)) { \
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
	value = (u_uint##funcIndex)tmpvalue;\
}while(0)

#define quick_parse_data(dataBuf, buf, bufLen, errorMsg) do{\
	if (ProtocolPacket::get_dataByLen(dataBuf, buf, bufLen)) {\
		logs(Logger::ERR, "%s", errorMsg); \
		return -1;\
	} \
}while(0)

#define quick_parse_unicodeString1BLen(dataBuf, str, errorMsg) do{\
	uif(ProtocolPacket::get_unicodeString1BLen(dataBuf, str)) {\
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_parse_unicodeString2BLen(dataBuf, str, errorMsg) do{\
	uif(ProtocolPacket::get_unicodeString2BLen(dataBuf, str)) {\
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_parse_unicodeWString2BLen(dataBuf, str, errorMsg) do{\
	uif(ProtocolPacket::get_unicodeWString2BLen(dataBuf, str)) {\
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_parse_unicodeStringByLen(dataBuf, len, str, errorMsg) do{\
	uif(ProtocolPacket::get_unicodeStringByLen(dataBuf, len, str)) {\
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_parse_stringBuf1BLen(dataBuf, str, errorMsg) do{\
	uif(ProtocolPacket::get_stringBuf1BLen(dataBuf, str)) {\
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

//get_unicodeStringByPosLen
#define quick_parse_unicodeStringByPosLen(dataBuf, str, pos, len, errorMsg) do{\
	uif(ProtocolPacket::get_unicodeStringByPosLen(dataBuf, str, pos, len)) {\
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_parse_stringBuf(packet, stringBuf, readLength, errorMsg) do{\
	uif (readLength < 0) {\
		logs(Logger::ERR, "read length (%d) < 0 error", readLength);\
		return -1;\
	}\
	stringBuf.reallocMem(stringBuf.length() + readLength);\
	quick_parse_data(packet, (u_uint8*)(stringBuf.addr() + stringBuf.length()), readLength, errorMsg); \
	stringBuf.set_length(stringBuf.length() + readLength);\
}while(0)

#define quick_check_dataLength(packet, needLength) do{\
	uif(packet.get_remailLength() < (unsigned int)needLength) { \
		logs(Logger::ERR, "remail length(%d) is too short, need length(%d)", packet.get_remailLength(), (int)needLength);\
		return -1;\
	}\
}while(0)

#define quick_skip_bytes(packet, skipLength) do{\
	uif(packet.get_remailLength() < (unsigned int)skipLength) {\
		logs(Logger::ERR, "skip %d failes, the remail length is too small", skipLength);\
		return -1;\
	}\
	packet.set_offset(packet.get_offset() + skipLength);\
}while(0)

#define quick_set_integer_LT(funcIndex, dataBuf, value, errorMsg) do{\
	if (ProtocolPacket::set_integerData##funcIndex##_LT(dataBuf, value)) { \
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_set_integer(funcIndex, dataBuf, value, errorMsg) do{\
	if (ProtocolPacket::set_integerData##funcIndex(dataBuf, value)) { \
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_set_data(dataBuf, buf, bufLen, errorMsg) do{\
	if (ProtocolPacket::set_dataByLen(dataBuf, (unsigned char*)buf, bufLen)) {\
		logs(Logger::ERR, "%s", errorMsg); \
		return -1;\
	} \
}while(0)

#define quick_reset_integer(dataBuf, size, value, position, errorMsg) do{\
	if (ProtocolPacket::set_integerDataByLen(dataBuf, size, value, position)) { \
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_reset_integer_LT(dataBuf, size, value, position, errorMsg) do{\
	if (ProtocolPacket::set_integerDataByLen_LT(dataBuf, size, value, position)) { \
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_set_stringUnicode1BLen(dataBuf, str, errorMsg) do{\
	uif(ProtocolPacket::set_stringUnicode1BLen(dataBuf, str)) {\
		logs(Logger::ERR, "%s", errorMsg);\
		return -1;\
	}\
}while(0)

#define quick_set_stringUnicode2BLen(dataBuf, str, errorMsg) do{\
	uif(ProtocolPacket::set_stringUnicode2BLen(dataBuf, str)) {\
		logs(Logger::ERR, "%s", errorMsg); \
		return -1;\
	}\
}while(0)

#define quick_set_wstringUnicode2BLen(dataBuf, str, errorMsg) do{\
	uif(ProtocolPacket::set_wstringUnicode2BLen(dataBuf, str)) {\
		logs(Logger::ERR, "%s", errorMsg); \
		return -1;\
	}\
}while(0)

#define quick_set_wstringUnicode1BLen(dataBuf, str, errorMsg) do{\
	uif(ProtocolPacket::set_wstringUnicode1BLen(dataBuf, str)) {\
		logs(Logger::ERR, "%s", errorMsg); \
		return -1;\
	}\
}while(0)


class ProtocolPacket {
public:
	ProtocolPacket();
	virtual ~ProtocolPacket();

	static int get_dataByLen(StringBuf& stringBuf, u_uint8* buf, unsigned int len);

	static int get_integerDataByLen(StringBuf& stringBuf, unsigned int size, u_uint64& value);
	static int get_integerData8(StringBuf& stringBuf, u_uint64& value);
	static int get_integerData16(StringBuf& stringBuf, u_uint64& value);
	static int get_integerData32(StringBuf& stringBuf, u_uint64& value);
	static int get_integerData64(StringBuf& stringBuf, u_uint64& value);

	static int get_integerDataByLen_LT(StringBuf& stringBuf, unsigned int size, u_uint64& value);
	static int get_integerData8_LT(StringBuf& stringBuf, u_uint64& value);
	static int get_integerData16_LT(StringBuf& stringBuf, u_uint64& value);
	static int get_integerData32_LT(StringBuf& stringBuf, u_uint64& value);
	static int get_integerData64_LT(StringBuf& stringBuf, u_uint64& value);

	static int get_unicodeString1BLen(StringBuf& stringBuf, std::string& desStr);
	static int get_unicodeString2BLen(StringBuf& stringBuf, std::string& desStr);
	static int get_unicodeWString2BLen(StringBuf& stringBuf, std::wstring& desWStr);
	static int get_unicodeStringByLen(StringBuf& stringBuf, const unsigned int len, std::string& desStr);
	static int get_stringBuf1BLen(StringBuf& stringBuf, StringBuf& desStr);
//
//	static int get_byteVarChar(StringBuf& stringBuf, std::string& desStr);
//	static int get_uVarChar(StringBuf& stringBuf, std::string& desStr);
//	static int get_usVarChar(StringBuf& stringBuf, std::string& desStr);

	static u_uint64 buffer2Integer(const void* buffer, const unsigned int len, bool is_littleEndian);
	static u_uint64 buffer2Integer(const void* buffer, const unsigned int len);

	static int get_string(StringBuf& stringBuf, std::string& desStr);//查找NULL结尾的字符串
	static int get_byteDataByPosLen(StringBuf& stringBuf, StringBuf& desStr,
			const unsigned int pos, const unsigned int len);
	static int get_unicodeStringByPosLen(StringBuf& stringBuf, std::string& desStr,
			const unsigned int pos, const unsigned int len);
	static int get_unicodeWStringByPosLen(StringBuf& stringBuf, std::wstring& desStr,
			const unsigned int pos, const unsigned int len);

	/////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
	static int set_dataByLen(StringBuf& stringBuf, u_uint8* buf, unsigned int len);

	static int set_integerDataByLen(StringBuf& stringBuf, unsigned int size, u_uint64 value);
	static int set_integerDataByLen(StringBuf& stringBuf, unsigned int size, u_uint64 value, int position);
	static int set_integerData8(StringBuf& stringBuf, u_uint64 value);
	static int set_integerData16(StringBuf& stringBuf, u_uint64 value);
	static int set_integerData32(StringBuf& stringBuf, u_uint64 value);
	static int set_integerData64(StringBuf& stringBuf, u_uint64 value);

	static int set_integerDataByLen_LT(StringBuf& stringBuf, unsigned int size, u_uint64 value);
	static int set_integerDataByLen_LT(StringBuf& stringBuf, unsigned int size, u_uint64 value, int position);
	static int set_integerData8_LT(StringBuf& stringBuf, u_uint64 value);
	static int set_integerData16_LT(StringBuf& stringBuf, u_uint64 value);
	static int set_integerData32_LT(StringBuf& stringBuf, u_uint64 value);
	static int set_integerData64_LT(StringBuf& stringBuf, u_uint64 value);

	static int set_stringUnicode1BLen(StringBuf& stringBuf, const std::string& desStr);
	static int set_stringUnicode2BLen(StringBuf& stringBuf, const std::string& desStr);
	static int set_wstringUnicode2BLen(StringBuf& stringBuf, const std::wstring& desStr);
	static int set_wstringUnicode1BLen(StringBuf& stringBuf, const std::wstring& desStr);
};

#endif /* PROTOCOL_PROTOCOLPACKET_H_ */
