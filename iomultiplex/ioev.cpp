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
 * @FileName: ioev.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年1月23日 下午4:03:12
 *  
 */

#include "ioev.h"
#include "logger.h"

#ifdef linux
IOEv::IOEv(std::string name): IoEvent(name) {
	// TODO Auto-generated constructor stub
	this->m_loop = ev_loop_new(0);
	this->m_timerId = 0;
}

IOEv::~IOEv() {
	// TODO Auto-generated destructor stub
	while(this->get_timerMap().size()) {
		IoEvent::TimerEventMapType::iterator it = this->get_timerMap().begin();
		this->del_timerEventInfo(it->first);
		ev_timer* et = (ev_timer*)it->first;
		delete et;
	}
}

void IOEv::stop_event() {
	ev_break(this->m_loop, EVBREAK_ALL);
	IoEvent::stop_event();
}

void IOEv::ioev_cb (EV_P_ ev_io *w, int revents)
{
	IOEv* ev = (IOEv*)w->data;
	const EventInfo* ioet = ev->get_IoEventInfo((int)w->fd);
	if (ioet == NULL) {
		logs(Logger::DEBUG, "not find fd(%d) in event map", w->fd);
		return;
	}
	IOEvFuncArgT* t = (IOEvFuncArgT*)ioet->func_param;
	ioet->fp(w->fd, ioet->event, t->func_args);
}

int IOEv::add_ioEvent(unsigned int fd, unsigned int event, Func func, void* args)
{
	ev_io* evio = NULL;
	EventInfo* ei = this->get_IoEventInfo(fd);
	if (ei) {
		IOEvFuncArgT* funcArgT = (IOEvFuncArgT*)ei->func_param;
		evio = (ev_io*)funcArgT->event;
		ev_io_stop(this->m_loop, evio);

		funcArgT->func_args = args;
		ei->event = event;
		ei->fp = func;
	} else {
		EventInfo eventInfo;
		IOEvFuncArgT* funcArgT = new IOEvFuncArgT();
		evio = new ev_io();

		funcArgT->event = evio;
		funcArgT->func_args = args;

		eventInfo.event = event;
		eventInfo.fp = func;
		eventInfo.func_param = funcArgT;

		this->add_ioEventInfo(fd, eventInfo);
	}

	ev_io_init(evio, IOEv::ioev_cb, fd, event);
	evio->data = this;
	ev_io_start(this->m_loop, evio);

	return 0;
}

int IOEv::add_ioEventRead(unsigned int fd, Func func, void* args)
{
	this->add_ioEvent(fd, EV_READ, func, args);
	return 0;
}

int IOEv::add_ioEventWrite(unsigned int fd, Func func, void *args)
{
	this->add_ioEvent(fd, EV_WRITE, func, args);
	return 0;
}

int IOEv::add_ioEventAccept(unsigned int fd, Func func, void *args)
{
	this->add_ioEvent(fd, EV_READ, func, args);
	return 0;
}

bool IOEv::is_readEvent(unsigned int event)
{
	if (event == EV_READ)
		return true;
	return false;
}

bool IOEv::is_writeEvent(unsigned int event)
{
	if (event == EV_WRITE)
		return true;
	return false;
}

void IOEv::del_ioEvent(unsigned int fd)
{
	EventInfo* eventInfo = this->get_IoEventInfo(fd);
	if (eventInfo == NULL)
		return;

	if (eventInfo->func_param != NULL) {
		IOEvFuncArgT* t = (IOEvFuncArgT*)eventInfo->func_param;
		if (t->event) {
			ev_io* ei = (ev_io*)t->event;
			ev_io_stop(this->m_loop, ei);
			delete ei;
		}
		delete t;
	}
	this->del_ioEventInfo(fd);
}

void IOEv::timerEv_cb(EV_P_ ev_timer *w, int revents)
{
	IOEv* ioev = (IOEv*)w->data;
	EventInfo* ei = ioev->get_timerEventInfo(w);
	if (ei == NULL) {
		logs(Logger::ERR, "get timer event error");
		return;
	}

	TimerFunc tf = (TimerFunc)ei->fp;
	tf(ei->func_param);

	if (fabs(ei->repeat) < 0.000001) {
		ioev->del_timerEventInfo(w);
		delete w;
	}
}

int IOEv::add_timerEvent(double after, double repeat, TimerFunc func, void* args)
{
	EventInfo event;
	event.event = 0;
	event.fp = (Func)func;
	event.func_param = args;
	event.after = after;
	event.repeat = repeat;

	ev_timer* timer = new ev_timer();
	this->add_timerEventInfo(timer, event);

	ev_timer_init(timer, IOEv::timerEv_cb, after, repeat);
	timer->data = this;
	ev_timer_start(this->m_loop, timer);
	return 0;
}

void IOEv::run_loop() {
	ev_run(this->m_loop, 0);
}

void IOEv::stop_loop() {
	ev_break(this->m_loop, EVBREAK_ALL);
}

void IOEv::regester_checkQuit() {
	this->add_timerEvent(1.0, 1.0, IoEvent::check_quit, this);
}
#endif
