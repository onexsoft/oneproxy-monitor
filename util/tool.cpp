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
* @FileName: tool.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月19日
*
*/
#include "tool.h"
#include "logger.h"
#include "systemapi.h"
#include "memmanager.h"
#include "convertutf.h"

#include <vector>
#include <cassert>
#include <stdarg.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#ifdef _WIN32
#include <io.h>
#include <dirent.h>
#else
#include <sys/io.h>
#include <unistd.h>
#include <dirent.h>
#endif

const int __const_litte_end = 0x12345678;
#define is_bigEnd() (((char*)(&__const_litte_end))[0] == 0x12)
unsigned int Tool::quick_hash_code(const char* data , const int len)
{
	#define tolower(x)   ((x)>= 'A' && (x) <= 'Z' ? (x) - 'A' + 'a' : (x))
	#define get16bits(x) (256 * tolower(*x) + tolower(*(x+1)))
	int tlen = len;
	unsigned int hash = tlen, tmp;
	int rem;

	if (tlen <= 0 || data == NULL) return 0;

	rem = tlen & 3;
	tlen >>= 2;

	/* Main loop */
	for (;tlen > 0; tlen--) {
	   hash  += get16bits (data);
	   tmp    = (get16bits (data+2) << 11) ^ hash;
	   hash   = (hash << 16) ^ tmp;
	   data  += 2*sizeof (unsigned short);
	   hash  += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
	   case 3: hash += get16bits (data);
			   hash ^= hash << 16;
			   hash ^= ((signed char)tolower(data[sizeof (unsigned short)])) << 18;
			   hash += hash >> 11;
			   break;
	   case 2: hash += get16bits (data);
			   hash ^= hash << 11;
			   hash += hash >> 17;
			   break;
	   case 1: hash += (signed char)tolower(*data);
			   hash ^= hash << 10;
			   hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

#undef tolower
	return hash;
}

int Tool::byte2string(const u_uint8* bdata, const unsigned int bdataLen, std::string& str)
{
	if (bdataLen <= 0)
		return 0;

	if (bdataLen%2 != 0) {
		return -1;
	}

	str.reserve(bdataLen/2 + 1);
	unsigned int i = 0;
	for (i = 0; i < bdataLen; i = i + 2) {
		str.insert(i/2, 1, (char)((bdata[i] & 0xff) | (bdata[i+1] & 0xff) << 8));
	}

	return 0;
}

int Tool::string2byte(const std::string& str, StringBuf& bdata)
{
	bdata.reallocMem(bdata.get_length() + str.length() * 2);
	for (unsigned int i = 0; i < str.length(); ++i) {
		char ch = (char)(*(str.c_str() + i));
		bdata.addr()[bdata.get_length() + i * 2] = (u_uint8)(ch & 0xff);
		bdata.addr()[bdata.get_length() + i * 2 + 1] = (u_uint8)((ch >> 8) & 0xff);
	}
	bdata.set_length(bdata.get_length() + str.length() * 2);
	return 0;
}

int Tool::byte2wstring(const u_uint8* bdata, const unsigned int bdataLen, std::wstring& wstr)
{
	if (bdataLen%2 != 0) {
		return -1;
	}

	wstr.reserve(bdataLen/2 + 1);
	unsigned int i = 0;
	for (i = 0; i < bdataLen; i = i + 2) {
		wstr.insert(i/2, 1, (wchar_t)((bdata[i] & 0x00ff) | (bdata[i+1] & 0x00ff) << 8));
	}
	return 0;
}

int Tool::wstring2byte(const std::wstring& wstr, StringBuf& bdata)
{
	bdata.reallocMem(bdata.get_length() + wstr.length() * 2);
	for (unsigned int i = 0; i < wstr.length(); ++i) {
		wchar_t ch = (wchar_t)(*(wstr.c_str() + i));
		bdata.addr()[bdata.get_length() + i * 2] = (u_uint8)(ch & 0xff);
		bdata.addr()[bdata.get_length() + i * 2 + 1] = (u_uint8)((ch >> 8) & 0xff);
	}
	bdata.set_length(bdata.get_length() + wstr.length() * 2);
	return 0;
}

std::string Tool::itoa(const int number)
{
	char buf[32];
	memset(buf, 0, 32);
	sprintf(buf, "%d", number);
	return std::string(buf, strlen(buf));
}

std::string Tool::format_string(const char* str, const unsigned int strLen, const unsigned int length)
{
	std::string tmpstr;
	if (strLen > length) {
		tmpstr = std::string(str, length - 3);
		tmpstr.append("...\0");
		return tmpstr;
	} else {
		tmpstr = std::string(str, strLen);
	}
	return tmpstr;
}

void Tool::tolower(std::string& str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void Tool::toupper(std::string& str)
{
	transform(str.begin(), str.end(), str.begin(),::toupper);
}

std::string Tool::args2string(const char* fmt, ...)
{
	std::string result;

	va_list args;
	va_start(args, fmt);
	Tool::argList2string(fmt, args, result);
	va_end(args);

	return result;
}

void Tool::argList2string(const char* fmt, va_list& argList, std::string& result)
{
	va_list tmpargs;
#ifndef va_copy
#define	va_copy(dst, src)	memcpy(&(dst), &(src), sizeof(va_list))
#endif
	char* buf = NULL;
	int len = 10;
	for(;;) {
		buf = (char*)MemManager::malloc(len);

		va_copy(tmpargs, argList);
		int sz = Tool::stringbuf_vsnprintf(buf, len, fmt, tmpargs);
		va_end(tmpargs);
		if (sz >= len) {
			len = sz + 1;
			if (buf) {
				MemManager::free(buf);
			}
			continue;
		} else if (sz < len) {
			result.append(buf, sz);
			MemManager::free(buf);
			break;
		}
	}
}

int Tool::stringbuf_vsnprintf(char *buf, size_t buflen, const char *format, va_list ap)
{
	int r;
	if (!buflen)
		return 0;

#if defined(_MSC_VER) || defined(WIN32)
	r = _vsnprintf(buf, buflen, format, ap);
	if (r < 0)
		r = _vscprintf(format, ap);
#elif defined(sgi)
	/* Make sure we always use the correct vsnprintf on IRIX */
	extern int      _xpg5_vsnprintf(char * __restrict,
		__SGI_LIBC_NAMESPACE_QUALIFIER size_t,
		const char * __restrict, /* va_list */ char *);

	r = _xpg5_vsnprintf(buf, buflen, format, ap);
#else
	r = vsnprintf(buf, buflen, format, ap);
#endif
	buf[buflen-1] = '\0';
	return r;
}

int Tool::string2wstring(const std::string& str, std::wstring& wstr)
{
	if (str.length() <= 0)
		return 0;
	for(unsigned int i = 0; i < str.length(); ++i) {
		wchar_t wc = (wchar_t)(*((char*)(str.c_str() + i)));
		wstr.insert(wstr.length(), 1, wc);
	}
	return 0;
}

int Tool::wstring2string(const std::wstring& wstr, std::string& str)
{
	if (wstr.length() <= 0)
		return 0;

	for (unsigned int i = 0; i < wstr.length(); ++i) {
		wchar_t wc = *(wstr.c_str() + i);
		char chh = (char)((wc >> 8) & 0x00ff);
		char chl = (char)(wc & 0x00ff);

		if (is_bigEnd()) {
			str.insert(str.length(), 1, chh);
			str.insert(str.length(), 1, chl);
		} else {
			str.insert(str.length(), 1, chl);
			str.insert(str.length(), 1, chh);
		}
	}
	return 0;
}

std::string Tool::int2string(int i)
{
	std::stringstream ss;
	std::string result;
	ss << i;
	ss >> result;
	return result;
}

std::string Tool::search_oneFile(std::string dirStr, std::string fileType)
{
	std::string result;
	if (unlikely(dirStr.length() <=0) || unlikely(fileType.length() <= 0)) {
		return result;
	}

#ifdef _WIN32
	int file = 0;
	struct _finddata_t fileInfo;
	std::string pathName;


	pathName.assign(dirStr).append("\\").append(fileType);
	if ((file = _findfirst(pathName.c_str(), &fileInfo)) == -1) {
		return result;
	 }


	 do
	 {
		 if (!(fileInfo.attrib & _A_SUBDIR)) {
			 result.append(dirStr).append("\\");
			 result.append(fileInfo.name);
			 break;
		 }
	 } while (_findnext(file, &fileInfo) == 0);
	 _findclose(file);
#else
    DIR *dir;
    struct dirent *ptr;

    //1. open dir
    if ((dir = opendir(dirStr.c_str())) == NULL)
	{
		logs(Logger::ERR, "open dir(%s) error(%s)", dirStr.c_str(),  SystemApi::system_strerror());
		return result;
	}

    //2. get destination file prefix
    std::string desFileType = fileType.substr(fileType.find_last_of('.'));
    if (desFileType.length() <= 0)
    	desFileType = fileType;

    //3. find destination file
    while ((ptr = readdir(dir)) != NULL)
    {
    	std::string fileName(ptr->d_name);
        if(ptr->d_type == 8 && (fileName.find(desFileType) + desFileType.length()) >= fileName.length()) {    ///file
            if(dirStr.find_last_of('/') != dirStr.npos) {
            	result.append(dirStr).append(fileName);
            } else {
            	result.append(dirStr).append("/").append(fileName);
            }
            closedir(dir);
            return result;
        }
    }
    closedir(dir);
#endif

	return result;
}

int Tool::find_sqlKeyWord(const std::string& src, const std::string& dst, bool ignoreCase)
{
	if (src.length() < dst.length())
		return -1;

	char* dch = (char*)dst.c_str();
	char* uch = NULL;
	char* lch = NULL;
	bool startCharacter = false;
	int findPos = -1;
	std::string upperStr = dst;
	std::string lowerStr = dst;
	unsigned int di = 0;

	if (ignoreCase) {
		Tool::toupper(upperStr);
		Tool::tolower(lowerStr);
		uch = (char*)upperStr.c_str();
		lch = (char*)lowerStr.c_str();
	}
	char* ch = (char*)src.c_str();
	unsigned int i = 0;

	for (i = 0; i < src.length(); ++i) {
		if (ch[i] == '\'' && (i == 0 || (ch[i-1] != '\\'))) {
			startCharacter = (startCharacter == false ? true : false);
			continue;
		}
		if (startCharacter)
			continue;

		if ((ignoreCase && (ch[i] == uch[di] || ch[i] == lch[di])) || (ch[i] == dch[di])) {
			if (findPos == -1)
				findPos = i;
			if ((di == (dst.length() - 1)) && ((i >= (src.length() - 1)) || src[i + 1] == ' ' || src[i + 1] == ';'))
				break;
			di = di + 1;
		} else {
			di = 0;
			findPos = -1;
		}
	}
	if (i >= src.length()) {
		findPos = -1;
	}
	return findPos;
}
