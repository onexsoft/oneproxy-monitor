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
* @FileName: epoll.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月31日
*
*/

#ifndef EPOLL_H_
#define EPOLL_H_

#include "logger.h"
#include "mutexlock.h"
#include "define.h"
#include "record.h"
#include "ioevent.h"
#include "systemapi.h"

#ifndef WIN32
#include <sys/epoll.h>
#endif
#include <map>
#include <string>

class Epoll : public IoEvent {
public:
	Epoll(std::string name);
	~Epoll();

	//event is EPOLLIN or EPOLLOUT
	virtual int add_ioEvent(unsigned int fd, unsigned int event, Func func, void* args);
	virtual int add_ioEventRead(unsigned int fd, Func func, void* args);
	virtual int add_ioEventWrite(unsigned int fd, Func func, void *args);
	virtual int add_ioEventAccept(unsigned int fd, Func func, void *args);
	virtual bool is_readEvent(unsigned int event);
	virtual bool is_writeEvent(unsigned int event);
	//remove fd event
	virtual void del_ioEvent(unsigned int fd);
	//run,ms
	virtual void run_loopWithTimeout(int epollTimeout);
private:
	void run_eventWait(int epollTimeout);

private:
	unsigned int trigerMethod;//EPOLLET or EPOLLLT, default EPOLLET
	unsigned int epfd;

	MutexLock mutexLock;

	const int maxEvents;
};

#endif /* EPOLL_H_ */
