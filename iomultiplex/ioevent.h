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
* @FileName: ioevent.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月16日
*
*/

#ifndef IOMULTIPLEX_IOEVENT_H_
#define IOMULTIPLEX_IOEVENT_H_

#include <string>
#include <iostream>
#include <map>

typedef void (*Func)(unsigned int fd, unsigned int events, void* args);

class EventInfo{
public:
	EventInfo() {
		this->fp = NULL;
		this->func_param = NULL;
		this->event = 0;
	}

	EventInfo(Func fp, void* func_param, unsigned int event) {
		this->fp = fp;
		this->func_param = func_param;
		this->event = event;
	}

	EventInfo(const EventInfo& eventInfo) {
		this->fp = eventInfo.fp;
		this->func_param = eventInfo.func_param;
		this->event = eventInfo.event;
	}

	EventInfo& operator= (const EventInfo& eventInfo) {
		this->fp = eventInfo.fp;
		this->func_param = eventInfo.func_param;
		this->event = eventInfo.event;
		return *this;
	}

	~EventInfo() {

	}

public:
	Func fp;
	void* func_param;
	unsigned int event;
};

class IoEvent {

public:
	typedef std::map<unsigned int, class EventInfo> IoEventMapType;

public:
	IoEvent(std::string eventEame) {
		this->eventName = eventEame;
		this->stopEpoll = false;
		this->eventMap.clear();
	}
	virtual ~IoEvent(){}

	static IoEvent* get_instance(std::string name);
	void destory_instance();

	std::string get_eventName() {
		return this->eventName;
	}

	bool is_stop() {
		return this->stopEpoll == true;
	}

	void stop_event() {
		this->stopEpoll = true;
	}

	void add_ioEventInfo(unsigned int fd, EventInfo& event, int& is_new);
	void del_ioEventInfo(unsigned int fd);
	const EventInfo* get_IoEventInfo(unsigned int fd);
	unsigned int get_ioEventInfoSize();
	const IoEvent::IoEventMapType& get_eventMap();

	virtual int add_ioEvent(unsigned int fd, unsigned int event, Func func, void* args) = 0;
	virtual int add_ioEventRead(unsigned int fd, Func func, void* args) = 0;
	virtual int add_ioEventWrite(unsigned int fd, Func func, void *args) = 0;
	virtual int add_ioEventAccept(unsigned int fd, Func func, void *args) = 0;
	virtual bool is_readEvent(unsigned int event) = 0;
	virtual bool is_writeEvent(unsigned int event) = 0;

	//remove fd event
	virtual void del_ioEvent(unsigned int fd) = 0;

	//run,ms
	virtual void run_loopWithTimeout(int epollTimeout) = 0;
	void run_loop();
	void run_loop(int epollTimeout);
	void run_once();

private:
	std::string eventName;
	bool stopEpoll;

	IoEvent::IoEventMapType eventMap;//key fd
};

#endif /* IOMULTIPLEX_IOEVENT_H_ */
