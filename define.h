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
* @FileName: define.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月1日
*
*/

#ifndef DEFINE_H_
#define DEFINE_H_

#include <iostream>
#include <map>

const int cf_tcp_socket_buffer = 0;
const int cf_tcp_keepidle = 0;
const int cf_tcp_keepintvl = 0;
const int cf_tcp_keepcnt = 0;

const int cf_listen_backlog = 128;

#define u_uint64 unsigned long long
#define u_uint8 unsigned char
#define u_uint16 unsigned short int
#define u_uint32 unsigned int
#define u_int32 int
#define u_int16 short int
#define u_int8 char
#define u_int64 long long

#define oneproxy_version() "v1.0.0"

//定义基本数据类型的类成员宏
#define declare_class_member(type, memberName) \
	private: type m_##memberName; \
	public: type get_##memberName(){ return m_##memberName;} \
	public: void set_##memberName(type value) {m_##memberName = value;}
//定义类对象的类成员宏
#define declare_class_member_co(type, memberName) \
	private: type m_##memberName; \
	public: type& get_##memberName(){ return m_##memberName;} \
	public: void set_##memberName(type& value) {m_##memberName = value;}

#define declare_type_alias(alias, ...) \
		typedef __VA_ARGS__ alias;
#define declare_clsfunc_pointer(retValType, className, funcPointerName, ...) \
		typedef retValType (className::*funcPointerName)(__VA_ARGS__);

#define strncmp_c(str1, str2) strncmp((const char*)str1, (const char*)str2, strlen(str2))
#define strncmp_s(str1, str2) strncmp(str1.c_str(), str2.c_str(), str2.length())
#define strncasecmp_c(str1, str2) strncasecmp((const char*)str1, (const char*)str2, strlen(str2))
#define strncasecmp_s(str1, str2) strncasecmp(str1.c_str(), str2.c_str(), str2.length())
#define cmpdata(type, first, oper, second) ((type)(first) oper (type)(second))

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)
#define uif(x) if (unlikely(x))
#define lif(x) if (likely(x))
#define assert(x) uif(!(x)){logs(Logger::FATAL, "assert %s failed", #x);}

#define forrange(var, start, end) for(var = start; var < end; ++var)
typedef void (*FreeFunc) (void*);

#ifdef _WIN32
#ifndef SIGUSR1
#define SIGUSR1 10
#endif
#ifndef SIGUSR2
#define SIGUSR2 12
#endif
#endif

typedef std::map<std::string, std::string> KVStringMap;

#endif /* DEFINE_H_ */
