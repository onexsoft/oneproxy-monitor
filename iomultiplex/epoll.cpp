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
* @FileName: epoll.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月31日
*
*/

#include "epoll.h"

Epoll::Epoll(std::string name)
	:IoEvent(name),
	mutexLock("_lock", record())
{
#ifndef WIN32
	epfd = epoll_create(256);//the max handles
	trigerMethod = EPOLLET;
#else
	epfd = 0;
	trigerMethod = 0;
#endif
	mutexLock.set_name(name + mutexLock.get_name());
}

Epoll::~Epoll()
{
#ifndef WIN32
	close(epfd);
#endif
}

int Epoll::add_ioEvent(unsigned int fd, unsigned int event, Func func, void *args)
{

	logs(Logger::INFO, "fd: %d, event: %d", fd, event);
#ifndef WIN32

	//when event is -1. represent only monitor the fd
	int is_new = 0;
	EventInfo eventInfo;

	eventInfo.event = event;
	eventInfo.fp = func;
	eventInfo.func_param = args;

	mutexLock.lock();
	this->add_ioEventInfo(fd, eventInfo, is_new);
	mutexLock.unlock();

	struct epoll_event ev;
	ev.data.ptr = NULL;
	ev.data.fd = fd;
	ev.events = this->trigerMethod | event;
	if (is_new) {
		if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev)) {
			logs(Logger::ERR, "add fd(%d) event(%d) error", fd, event);
		}
	} else {
		if (epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev)) {
			logs(Logger::ERR, "mod fd(%d) event(%d) error", fd, event);
		}
	}
#endif
	return 0;
}

int Epoll::add_ioEventRead(unsigned int fd, Func func, void* args)
{
#ifdef linux
	return this->add_ioEvent(fd, (unsigned int)EPOLLIN, func, args);
#else
	return 0;
#endif
}

int Epoll::add_ioEventWrite(unsigned int fd, Func func, void *args)
{
#ifdef linux
	return this->add_ioEvent(fd, (unsigned int)EPOLLOUT, func, args);
#else
	return 0;
#endif
}

int Epoll::add_ioEventAccept(unsigned int fd, Func func, void *args)
{
	return this->add_ioEvent(fd, (unsigned int)-1, func, args);
}

bool Epoll::is_readEvent(unsigned int event)
{
#ifdef linux
	return (bool)(event & EPOLLIN);
#else
	return false;
#endif
}

bool Epoll::is_writeEvent(unsigned int event)
{
#ifdef linux
	return (bool)(event & EPOLLOUT);
#else
	return false;
#endif
}

void Epoll::del_ioEvent(unsigned int fd)
{
	logs(Logger::INFO, "delete fd(%d) epoll event", fd);
	//1. delete from epoll
#ifdef linux
	struct epoll_event ev;
	ev.data.fd = fd;
	if (epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, &ev)) {
		logs(Logger::ERR, "del fd(%d) error", fd);
	}
#endif
	//2. delete from eventMap
	mutexLock.lock();
	this->del_ioEventInfo(fd);
	mutexLock.unlock();
}

void Epoll::run_loopWithTimeout(int epollTimeout)
{
	run_eventWait(epollTimeout);
}

void Epoll::run_eventWait(int epollTimeout)
{
#ifdef linux
	const int maxEvents = 20;
	int nfds = 0;
	struct epoll_event events[maxEvents];

	nfds = epoll_wait(this->epfd, events, maxEvents, epollTimeout);
	for (int i = 0; i < nfds; ++i) {

		const EventInfo* ioet = this->get_IoEventInfo((int)events[i].data.fd);
		if (ioet == NULL) {
			logs(Logger::INFO, "not find fd(%d) in event map", events[i].data.fd);
			continue;
		}

		if (ioet->event == (unsigned int)(-1)) {
			ioet->fp(events[i].data.fd, events[i].events, ioet->func_param);
			this->add_ioEventAccept(events[i].data.fd, ioet->fp, ioet->func_param);

		} else if ((ioet->event & events[i].events) && (ioet->event == EPOLLIN || ioet->event == EPOLLOUT)) {
			ioet->fp(events[i].data.fd, events[i].events, ioet->func_param);
		}
	}
#endif
}
