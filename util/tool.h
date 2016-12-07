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
* @FileName: tool.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月19日
*
*/

#ifndef PROTOCOL_TOOL_H_
#define PROTOCOL_TOOL_H_

#include "define.h"
#include "stringbuf.h"

#include <string>
#include <stdarg.h>
#include <iostream>

class Tool{
public:

	static unsigned int quick_hash_code(const char* string, const int len);

	static int byte2string(const u_uint8* bdata, const unsigned int bdataLen, std::string& str);
	static int string2byte(const std::string& str, StringBuf& bdata);
	static int byte2wstring(const u_uint8* bdata, const unsigned int bdataLen, std::wstring& wstr);
	static int wstring2byte(const std::wstring& wstr, StringBuf& bdata);

	static std::string itoa(const int number);

	static std::string format_string(const char* str, const unsigned int strLen, const unsigned int length);
	static void tolower(std::string& str);
	static void toupper(std::string &str);
	static std::string args2string(const char* fmt, ...);
	static void argList2string(const char* fmt, va_list& argList, std::string& result);
	static int stringbuf_vsnprintf(char *buf, size_t buflen, const char *format, va_list ap);
	static int string2wstring(const std::string& str, std::wstring& wstr);
	static int wstring2string(const std::wstring& wstr, std::string& str);
	static std::string int2string(int i);

	/*
	 * @desc 查找指定目录下的一个文件，如果有多个目标文件，指需要返回第一个目标文件即可
	 * @param dir 指定的目录
	 *        fileType 文件类型，比如：*.txt, *.exe, *.ini
	 * @return 查找到的文件名称
	 * */
	static std::string search_oneFile(std::string dir, std::string fileType);

	//在src中查找dst,根据ignoreCase来决定是否忽略大小写,返回在src中的位置，如果不存在则返回-1
	static int find_sqlKeyWord(const std::string& src, const std::string& dst, bool ignoreCase);
};

#endif /* PROTOCOL_TOOL_H_ */
