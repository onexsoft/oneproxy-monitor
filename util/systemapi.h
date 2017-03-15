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
* @FileName: systemapi.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月17日
*
*/

#ifndef UTIL_SYSTEMAPI_H_
#define UTIL_SYSTEMAPI_H_

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#ifndef _WINSOCK2_H
#define _WINSOCK2_H
#endif
#include <ws2tcpip.h>

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/time.h>
#include <net/if.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <iostream>
#include <list>

#include "define.h"

#ifdef _WIN32
#define SystemSocket SOCKET
#define system_ioctl ioctlsocket
#else
#define SystemSocket int
#define system_ioctl ioctl
#endif

class SystemApi{
public:
	static int getaddrinfo(const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result);
	static void freeaddrinfo(struct addrinfo *ai);
	static void close(int fd);

	static int system_errno();
	static char* system_strerror(int errorno);
	static char* system_strerror();
	static time_t system_time();
	static struct tm* system_timeTM();
	static std::string system_timeStr();
	static std::string system_time2Str(time_t t);
	static u_uint64 system_millisecond();
	static u_uint64 system_second();
	static int system_limitFileNum(unsigned int& fileNum);
	static unsigned int system_fdSetSize();

	static void system_sleep(int ms);

	//初始化网络环境，在创建socket之前进行调用
	static int init_networkEnv();

	//清理网络环境，在完成网络后调用
	static void clear_networkEnv();

	//获取当前进程的进程pid。
	static int get_pid();
	//获取系统cpu的个数
	static unsigned int system_cpus();

	//设置套接字的接收超时时间
	static int system_setSocketRcvTimeo(SystemSocket& socket, int second, int microsecond);
	static int system_setSocketSndTimeo(SystemSocket& socket, int second, int microsecond);
	static int system_setSockNonblocking(unsigned int fd, bool nonBlocking);
	static int system_inetPton(int af, const char* src, void* dst);
	static int system_ntop(int af, const void *src, char *dst, socklen_t cnt);
	static void system_setThreadName(std::string name);
	//设置文件描述符限制
	static int system_setFDNum(unsigned int max_files_number);
	static int system_socketpair(int family, int type, int protocol, int fd[2]);
	static void system_showNetworkInterfInfo();
	static std::string system_getIp(std::string device);
	static std::string system_getIpOnLinux(std::string device);
	static std::string system_getIpOnWindows(std::string device);
	static std::string system_getDeivceName(std::string ip);
	static int system_getIpList(std::string device, std::list<std::string>& ipList);

};

#endif /* UTIL_SYSTEMAPI_H_ */
