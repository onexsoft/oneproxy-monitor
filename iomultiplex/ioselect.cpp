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

typedef struct _select_timer_s{
	u_uint64 timer_time;
	bool isRepeat;
}SelectTimerS;

#define ONE_SECOND 1000

IoSelect::IoSelect(std::string name)
	:IoEvent(name)
{
	mutexLock.set_name(name + mutexLock.get_name());
	this->maxfd = 0;
	this->loopMinTime = 1000000;
	this->fdsetSet.clear();
	FD_ZERO(&this->readFdSet);
	FD_ZERO(&this->writeFdSet);
}

IoSelect::~IoSelect()
{
	while(this->get_timerMap().size()) {
		IoEvent::TimerEventMapType::iterator it = this->get_timerMap().begin();
		this->del_timerEventInfo(it->first);
		SelectTimerS* et = (SelectTimerS*)it->first;
		delete et;
	}
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
	if (event == SELECT_EVENT_WRITE) {
		FD_SET(fd, &this->writeFdSet);
	} else {
		FD_SET(fd, &this->readFdSet);
	}
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
	EventInfo* ei = this->get_IoEventInfo(fd);
	if (ei->event == SELECT_EVENT_WRITE) {
		FD_CLR(fd, &this->writeFdSet);
	} else {
		FD_CLR(fd, &this->readFdSet);
	}
	this->del_ioEventInfo(fd);
	this->fdsetSet.erase(fd);
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

//after and repeat all is second.
int IoSelect::add_timerEvent(double after, double repeat, TimerFunc func, void* args)
{
	EventInfo event;
	event.event = 0;
	event.fp = (Func)func;
	event.func_param = args;
	event.after = after;
	event.repeat = repeat;

	SelectTimerS* timer = new SelectTimerS();
	timer->timer_time = SystemApi::system_millisecond() + (int)(after * ONE_SECOND);
	this->add_timerEventInfo(timer, event);

	if (fabs(repeat) < 0.00001) {
		timer->isRepeat = false;
	} else {
		timer->isRepeat = true;
	}

	int tmpAfter = (int)(after * ONE_SECOND);
	int tmpRepeat = (int)(repeat * ONE_SECOND);
	int tmpMinTime = tmpAfter > tmpRepeat ? (tmpRepeat > 0 ? tmpRepeat : tmpAfter)
			: (tmpAfter > 0 ? tmpAfter : tmpRepeat);
	if (tmpMinTime < this->loopMinTime || this->loopMinTime <= 0) {
		if (tmpMinTime > 0) {
			this->loopMinTime = tmpMinTime;
		}
	}

	return 0;
}

void IoSelect::run_loop() {
	while(this->is_stop() == false) {
		this->run_loopWithTimeout(this->loopMinTime);
	}
}

void IoSelect::regester_checkQuit() {
	this->add_timerEvent(1.0, 1.0, IoEvent::check_quit, this);
}

//run,ms
void IoSelect::run_loopWithTimeout(int timeout)
{
	struct timeval tt;
	tt.tv_sec = timeout/1000;
	tt.tv_usec = (timeout%1000) * 1000;
	if (this->get_eventMap().size() > 0) {
		fd_set readSet, writeSet;
		readSet = this->readFdSet;
		writeSet = this->writeFdSet;
		int ret = 0;
		do {
			ret = select (this->maxfd + 1, &readSet, &writeSet, NULL, &tt);
			if (ret < 0)
				errno = SystemApi::system_errno();
		} while(ret < 0 && errno == EINTR);

		if (ret < 0) {
			errno = SystemApi::system_errno();
			logs(Logger::ERR, "maxfd: %d, select(%s) error(%d:%s)",this->maxfd, this->get_eventName().c_str(),
					errno, SystemApi::system_strerror(errno));
			return;
		} else if (ret > 0) {
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
	} else {
		int ret = 0;
		do {
			ret = select(0, NULL, NULL, NULL, &tt);
			if (ret < 0)
				errno = SystemApi::system_errno();
		} while(ret < 0 && errno == EINTR);
	}

	IoEvent::TimerEventMapType& timerEventMap = this->get_timerMap();
	IoEvent::TimerEventMapType::iterator it = timerEventMap.begin();
	u_uint64 current_time = SystemApi::system_millisecond();
	for(; it != timerEventMap.end(); ++it) {
		SelectTimerS* sts = (SelectTimerS*)it->first;
		EventInfo& ei = it->second;
		if (current_time >= sts->timer_time) {
			TimerFunc tf = (TimerFunc)ei.fp;
			tf(ei.func_param);

			if (!sts->isRepeat) {
				this->del_timerEventInfo(sts);
				delete sts;
			} else {
				sts->timer_time = current_time + (int)(ei.repeat * ONE_SECOND);
			}
		}
	}
}
