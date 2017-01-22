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
 * @FileName: socketutil.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年1月20日 上午10:35:05
 *  
 */

#include "socketutil.h"
#include "logger.h"
#include "systemapi.h"
#include <errno.h>

int SocketUtil::socket_writeData(int fd, const void* data, const unsigned int dataLen, int microsecond)
{
	int writedLen = 0;

	if (data == NULL || dataLen <= 0)
		return 0;

	if (microsecond <= 0)
		return -1;

	do {
		int ret = write(fd, (const void*)((char*)data + writedLen), dataLen - writedLen);
		if (ret < 0) {
			errno = SystemApi::system_errno();
#ifdef _WIN32
			if (errno == WSAEWOULDBLOCK) {
				SystemApi::system_sleep(microsecond);
				continue;
			}
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				SystemApi::system_sleep(microsecond);
				continue;
			}
			return -1;
#endif
		} else if ((ret + writedLen) >= (int)dataLen) {
			return 0;
		}
		writedLen += ret;
	} while(1);

	return 0;
}

int SocketUtil::socket_readData(int fd, void* data, const unsigned int dataLen, int microsecond)
{
	if (data == NULL || dataLen <= 0)
		return -1;

	do{
		int ret = read(fd, data, dataLen);
		if (ret < 0) {
			errno = SystemApi::system_errno();
#ifdef _WIN32
			if (errno == WSAEWOULDBLOCK) {
				SystemApi::system_sleep(microsecond);
				continue;
			}
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				SystemApi::system_sleep(microsecond);
				continue;
			}
			return -1;
#endif
		} else {
			return ret;
		}
	}while(1);

	return 0;
}

int SocketUtil::socket_readAllData(int fd, StringBuf& sb, int microsecond)
{
	unsigned int dataLen = 0;
	while(true) {
		if (system_ioctl(fd, FIONREAD, (unsigned long int*)&dataLen)) {//fd maybe closed
			logs(Logger::ERR, "fd(%d) ioctl FIONREAD error", fd);
			return -1;
		}
		if (dataLen <= 0) {
			return 0;
		}

		sb.reallocMem(sb.get_length() + dataLen);
		int len = read(fd, (char*)(sb.addr() + sb.length()), dataLen);
		if (len == -1) {
			errno = SystemApi::system_errno();
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				SystemApi::system_sleep(microsecond);
				continue;
			}
			logs(Logger::ERR, "recv data from fd(%d) error(%d:%s)", fd,
					errno, SystemApi::system_strerror(errno));
			return -1;
		}
		sb.set_length(sb.length() + len);
		break;
	}
	return 0;
}
