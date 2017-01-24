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
 * @FileName: ioev.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年1月23日 下午4:03:11
 *  
 */

#ifndef IOMULTIPLEX_IOEV_H_
#define IOMULTIPLEX_IOEV_H_
#include "ioevent.h"
#include <ev.h>

typedef struct _ioev_func_args_t{
	void* func_args;
	void* event;
}IOEvFuncArgT;

class IOEv : public IoEvent{
public:
	IOEv(std::string name);
	virtual ~IOEv();
	virtual void stop_event();

	static void ioev_cb (EV_P_ ev_io *w, int revents);
	virtual int add_ioEvent(unsigned int fd, unsigned int event, Func func, void* args);
	virtual int add_ioEventRead(unsigned int fd, Func func, void* args);
	virtual int add_ioEventWrite(unsigned int fd, Func func, void *args);
	virtual int add_ioEventAccept(unsigned int fd, Func func, void *args);
	virtual bool is_readEvent(unsigned int event);
	virtual bool is_writeEvent(unsigned int event);
	virtual void del_ioEvent(unsigned int fd);//remove fd event

	static void timerEv_cb(EV_P_ ev_timer *w, int revents);
	virtual int add_timerEvent(double after, double repeat, TimerFunc func, void* args);

	//run,ms
	virtual void run_loopWithTimeout(int epollTimeout);
	virtual void run_loop();
	virtual void run_once();
	virtual void stop_loop();
private:
	struct ev_loop *m_loop;
	int m_timerId;
};

#endif /* IOMULTIPLEX_IOEV_H_ */
