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
* @FileName: stringbuf.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月9日
*
*/
#include "stringbuf.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tool.h"
#include "logger.h"
#include "memmanager.h"

#define default_cache_size (1024 * 1024)
#define default_max_cache_size (10 * default_cache_size)

StringBuf::StringBuf()
{
	this->m_allocateLen = 0;
	this->m_length = 0;
	this->m_offset = 0;
	this->m_buf = NULL;
}

StringBuf::StringBuf(StringBuf const& sb)
{
	this->m_offset = sb.m_offset;
	this->m_buf = (char*)MemManager::malloc(sb.m_allocateLen);
	uif (this->m_buf == NULL)
		return;
	memmove(this->m_buf, sb.m_buf, sb.m_length);
	this->m_length = sb.m_length;
	this->m_allocateLen = sb.m_allocateLen;
}

StringBuf::~StringBuf() {
	if (this->m_buf != NULL) {
		MemManager::free(this->m_buf);
	}
}

int StringBuf::reallocMem(unsigned int size)
{
	if (this->m_allocateLen >= size)//memory sufficient
		return 0;

	this->m_allocateLen = this->alignment(size);
	char* tmpbuf = (char*)MemManager::malloc(this->m_allocateLen);
	uif (tmpbuf == NULL) {
		logs(Logger::ERR, "malloc(%d) error", this->m_allocateLen);
		return -1;
	}

	uif(this->m_length > this->m_allocateLen) {//error
		logs(Logger::ERR, "m_length(%u) > m_allocateLen(%u), don't memmove", this->m_length, this->m_allocateLen);
		return -1;
	}
	memmove((void*)tmpbuf, (const void*)this->m_buf, this->m_length);
	lif (this->m_buf) {
		MemManager::free(this->m_buf);
		this->m_buf = NULL;
	}
	this->m_buf = tmpbuf;
	return 0;
}

void StringBuf::mallocMem(unsigned int size)
{
	if (this->m_allocateLen >= size)
			return;

	if (this->m_buf != NULL) {
		MemManager::free(this->m_buf);
	}

	this->m_allocateLen = this->alignment(size);
	this->m_buf = (char*)MemManager::malloc(this->m_allocateLen);
	this->m_length = 0;
	this->m_offset = 0;
}

unsigned int StringBuf::alignment(unsigned int size, unsigned int alignmentLength)
{
	size = size > 0 ? size : 0;
	size = (size + alignmentLength - 1) & (~(alignmentLength-1));
	return size;
}

int StringBuf::append(const char* str)
{
	if (this->m_allocateLen < this->m_length + strlen(str)) {
		this->m_allocateLen = this->alignment(this->m_length + strlen(str));
		char* tmpbuf = (char*)MemManager::malloc(this->m_allocateLen);
		memmove((void*)tmpbuf, this->m_buf, this->m_length);
		memmove((void*)(tmpbuf + this->m_length), str, strlen(str));
		if (this->m_buf) {
			MemManager::free(this->m_buf);
			this->m_buf = NULL;
		}
		this->m_buf = tmpbuf;
		this->m_length = this->m_length + strlen(str);
		this->m_offset = 0;
		return 0;
	}
	memmove((void*)(this->m_buf + this->m_length), (void*)str, strlen(str));
	this->m_length += strlen(str);
	return 0;
}

int StringBuf::append(const void* data, int dataLen)
{
	if (this->m_allocateLen < this->m_length + dataLen) {
		this->m_allocateLen = this->alignment(this->m_length + dataLen);
		char* tmpbuf = (char*)MemManager::malloc(this->m_allocateLen);
		memmove((void*)tmpbuf, this->m_buf, this->m_length);
		memmove((void*)(tmpbuf + this->m_length), data, dataLen);
		if (this->m_buf) {
			MemManager::free(this->m_buf);
			this->m_buf = NULL;
		}
		this->m_buf = tmpbuf;
		this->m_length = this->m_length + dataLen;
		return 0;
	}

	memmove((void*)(this->m_buf + this->m_length), data, dataLen);
	this->m_length += dataLen;
	return 0;
}

int StringBuf::appendFormat(const char* fmt, ...)
{
	std::string result;
	va_list args;
	va_start(args, fmt);
	Tool::argList2string(fmt, args, result);
	this->append(result.c_str());
	va_end(args);
	return 0;
}

int StringBuf::insert(const unsigned int pos, const void* data, const int dataLen)
{
	if (pos > this->m_length) {
		int zeroLen = pos - this->m_length;
		this->m_allocateLen = this->alignment(this->m_length + dataLen + zeroLen);
		char* tmpbuf = (char*)MemManager::malloc(this->m_allocateLen);
		memmove((void*)tmpbuf, this->m_buf, this->m_length);
		memmove((void*)(tmpbuf + pos), data, dataLen);
		if (this->m_buf) {
			MemManager::free(this->m_buf);
			this->m_buf = NULL;
		}
		this->m_buf = tmpbuf;
		this->m_length = this->m_length + dataLen + zeroLen;
		this->m_offset = 0;
		return 0;
	}

	if (this->m_allocateLen < this->m_length + dataLen) {
		this->m_allocateLen = this->alignment(this->m_length + dataLen);
		char* tmpbuf = (char*)MemManager::malloc(this->m_allocateLen);
		memmove((void*)tmpbuf, this->m_buf, pos);
		memmove((void*)(tmpbuf + pos), data, dataLen);
		memmove((void*)(tmpbuf + pos + dataLen), this->m_buf + pos, this->m_length - pos);
		if (this->m_buf) {
			MemManager::free(this->m_buf);
			this->m_buf = NULL;
		}
		this->m_buf = tmpbuf;
		this->m_length = this->m_length + dataLen;
		this->m_offset = 0;
		return 0;
	}

	memmove((void*)(this->m_buf + pos + dataLen), this->m_buf + pos, this->m_length - pos);
	memmove((void*)(this->m_buf + pos), data, dataLen);
	this->m_length += dataLen;
	return 0;
}

void StringBuf::erase(unsigned int startPos, unsigned int endPos)
{
	if (endPos > this->m_length) {
		endPos = this->m_length;
	}

	if (endPos <= startPos)
		return;

	//删除区间为前闭后开区间
	unsigned int movLen = this->m_length - endPos;
	unsigned int newLen = startPos + movLen;
	memmove((void*)(this->m_buf + startPos), (void*)(this->m_buf + endPos), movLen);
	this->set_length(newLen);
}

void StringBuf::clear()
{
	this->m_offset = 0;
	this->m_length = 0;

	if (this->m_allocateLen > default_max_cache_size) {//防止占用内存过大。
		if (this->m_buf) {
			unsigned int tsize = this->alignment(default_cache_size);
			char* tbuf = (char*)MemManager::malloc(tsize);
			if (tbuf != NULL) {
				MemManager::free(this->m_buf);
				this->m_buf = tbuf;
				this->m_allocateLen = tsize;
			}
		}
	}
}
