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
* @FileName: stringbuf.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月9日
*
*/

#ifndef UTIL_STRINGBUF_H_
#define UTIL_STRINGBUF_H_

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

#include "define.h"

class StringBuf{
public:
	StringBuf();
	StringBuf(StringBuf const&);
	~StringBuf();

	int reallocMem(unsigned int size);
	void mallocMem(unsigned int size);
	void erase(unsigned int startPos, unsigned int endPos);

	int append(const char* str);
	int append(const void* data, const int dataLen);
	int appendFormat(const char* fmt, ...);
	int insert(const unsigned int pos, const void* data, const int dataLen);
	void clear();

	inline char* addr() {
		return this->m_buf;
	}

	inline char* get_offsetAddr() {
		return (char*)(this->m_buf + this->m_offset);
	}

	inline unsigned int get_remailLength() {
		if (this->m_length >= this->m_offset) {
			return this->m_length - this->m_offset;
		}
		return 0;
	}

	inline unsigned int get_remailAllocLen() {
		if (this->m_length <= this->m_allocateLen)
			return this->m_allocateLen - this->m_length;
		return 0;
	}

	unsigned int length() {
		return this->m_length;
	}

private:
	unsigned int alignment(unsigned int size, unsigned int alignmentLength = 8);

private:
	declare_class_member(char*, buf)
	declare_class_member(unsigned int, length)
	declare_class_member(unsigned int, offset)
	declare_class_member(unsigned int, allocateLen)
};
#endif /* UTIL_STRINGBUF_H_ */
