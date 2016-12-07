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
* @FileName: ioselect.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月17日
*
*/

#include "ioselect.h"
#include "record.h"
#include <errno.h>
#include <string.h>
#include "systemapi.h"

typedef enum select_event_type_t{
	SELECT_EVENT_READ,
	SELECT_EVENT_WRITE,
	SELECT_EVENT_ACCEPT
}SelectEventType;
IoSelect::IoSelect(std::string name)
	:IoEvent(name)
{
	mutexLock.set_name(name + mutexLock.get_name());
	this->maxfd = 0;
	this->fdsetSet.clear();
	FD_ZERO(&this->fdset);
}

IoSelect::~IoSelect()
{
}

int IoSelect::add_ioEvent(unsigned int fd, unsigned int event, Func func, void* args)
{
	EventInfo eventInfo;
	eventInfo.event = event;
	eventInfo.fp = func;
	eventInfo.func_param = args;

	int is_new = 0;

	//检查已经增加的数量
#ifdef WIN32
	if (this->get_eventMap().size() >= (unsigned int)SystemApi::system_fdSetSize()) {//windows平台是 限制文件描述符的个数
		logs(Logger::INFO, "the socket is full, size: %d", this->get_eventMap().size());
		return -1;
	}
#else
	if (fd >= (unsigned int)SystemApi::system_fdSetSize()) {//linux平台是限制文件描述符的值
		logs(Logger::INFO, "the fd(%d) is too bigger, fd_setSize: %d", fd, SystemApi::system_fdSetSize());
		return -1;
	}
#endif
	mutexLock.lock();
	this->add_ioEventInfo(fd, eventInfo, is_new);
	FD_SET(fd, &this->fdset);
	this->fdsetSet.insert(fd);
	if (fd > this->maxfd)
		this->maxfd = fd;
	mutexLock.unlock();
	return 0;
}

int IoSelect::add_ioEventRead(unsigned int fd, Func func, void* args)
{
	return this->add_ioEvent(fd, SELECT_EVENT_READ, func, args);
}

int IoSelect::add_ioEventWrite(unsigned int fd, Func func, void *args)
{
	return this->add_ioEvent(fd, SELECT_EVENT_WRITE, func, args);
}

int IoSelect::add_ioEventAccept(unsigned int fd, Func func, void *args)
{
	logs(Logger::INFO, "add accept fd(%d) to event", fd);
	return this->add_ioEvent(fd, SELECT_EVENT_ACCEPT, func, args);
}

bool IoSelect::is_readEvent(unsigned int event)
{
	if (event == SELECT_EVENT_READ) {
		return true;
	}
	return false;
}

bool IoSelect::is_writeEvent(unsigned int event)
{
	if (event == SELECT_EVENT_WRITE)
		return true;
	return false;
}

//remove fd event
void IoSelect::del_ioEvent(unsigned int fd)
{
	mutexLock.lock();
	this->del_ioEventInfo(fd);
	this->fdsetSet.erase(fd);
	FD_CLR(fd, &this->fdset);
	if (fd >= this->maxfd) {
		if (this->fdsetSet.size() <= 0) {
			this->maxfd = 0;
		} else {
			this->maxfd = *(this->fdsetSet.begin());
		}
	}
	logs(Logger::INFO, "fdsetSet erase fd(%d)", fd);
	mutexLock.unlock();
}

//run,ms
void IoSelect::run_loopWithTimeout(int timeout)
{
	fd_set readSet, writeSet;
	struct timeval time;
	time.tv_sec = 0;
	time.tv_usec = timeout;

	if (this->get_eventMap().size() <= 0) {
		if (timeout > 0) {
			SystemApi::system_sleep(timeout);
		}
		return;
	}
	readSet = this->fdset;
	writeSet = this->fdset;

	int ret = 0;
	if (timeout == 0) {
		ret = select (this->maxfd + 1, &readSet, &writeSet, NULL, 0);
	} else if (timeout == -1) {
		ret = select (this->maxfd + 1, &readSet, &writeSet, NULL, NULL);
	} else {
		ret = select (this->maxfd + 1, &readSet, &writeSet, NULL, &time);
	}

	if (ret < 0) {
		errno = SystemApi::system_errno();
		logs(Logger::ERR, "maxfd: %d, select(%s) error(%d:%s)",this->maxfd, this->get_eventName().c_str(),
				errno, SystemApi::system_strerror(errno));
		return;
	} else if (ret == 0) {//表示没有socket可读写
		return;
	}

	IoEvent::IoEventMapType eventMap = this->get_eventMap();
	IoEvent::IoEventMapType::iterator it = eventMap.begin();
	for (; it != eventMap.end(); ++it) {
		unsigned int fd = it->first;
		if ((FD_ISSET(fd, &readSet) && (it->second.event == SELECT_EVENT_READ || it->second.event == SELECT_EVENT_ACCEPT))
				|| (FD_ISSET(fd, &writeSet) && it->second.event == SELECT_EVENT_WRITE)) {
			it->second.fp(fd, it->second.event, it->second.func_param);
		}
	}
}
