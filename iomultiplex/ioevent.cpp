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
* @FileName: ioevent.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月16日
*
*/

#include "ioevent.h"

#include "logger.h"
#include "epoll.h"
#include "ioselect.h"

IoEvent* IoEvent::get_instance(std::string name)
{
#ifdef linux
	return (IoEvent*)new Epoll(name);
#else
	return (IoEvent*)new IoSelect(name);
#endif
}

void IoEvent::destory_instance()
{
	delete this;
}

void IoEvent::run_loop(int epollTimeout) {
	while(false == this->is_stop()) {
		this->run_loopWithTimeout(epollTimeout);
	}
}

void IoEvent::run_loop() {
	while(false == this->is_stop()) {
		this->run_loopWithTimeout(-1);
	}
}

void IoEvent::run_once() {
	this->run_loopWithTimeout(0);
}

void IoEvent::add_ioEventInfo(unsigned int fd, EventInfo& event, int& is_new) {
	IoEvent::IoEventMapType::iterator it = this->eventMap.find(fd);
	if (it != this->eventMap.end()) {
		is_new = 0;
	} else {
		is_new = 1;
	}
	this->eventMap[fd] = event;
}

const EventInfo* IoEvent::get_IoEventInfo(unsigned int fd) {
	IoEvent::IoEventMapType::iterator it = this->eventMap.find(fd);
	if (it == this->eventMap.end())
		return NULL;
	return &it->second;
}

unsigned int IoEvent::get_ioEventInfoSize() {
	return this->eventMap.size();
}

const IoEvent::IoEventMapType& IoEvent::get_eventMap() {
	return this->eventMap;
}

bool IoEvent::is_eventExisted(unsigned int fd, EventInfo event)
{
	const EventInfo* ei = this->get_IoEventInfo(fd);
	if (ei != NULL && (*ei) == event) {
		return true;
	}
	return false;
}

void IoEvent::del_ioEventInfo(unsigned int fd) {
	logs(Logger::DEBUG, "del io event info(%u)", fd);
	IoEvent::IoEventMapType::iterator it = this->eventMap.find(fd);
	if (it == this->eventMap.end()) {
		logs(Logger::ERR, "not found fd(%d) in event map", fd);
		return;
	}
	this->eventMap.erase(it);
}

bool IoEvent::is_regesterEvent(unsigned int fd) {
	IoEvent::IoEventMapType::iterator it = this->eventMap.find(fd);
	if (it == this->eventMap.end()) {
		return false;
	}
	return true;
}
