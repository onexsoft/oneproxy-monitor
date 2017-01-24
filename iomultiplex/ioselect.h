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
* @FileName: ioselect.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月17日
*
*/

#ifndef IOMULTIPLEX_IOSELECT_H_
#define IOMULTIPLEX_IOSELECT_H_

#include "ioevent.h"
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <set>
#include "mutexlock.h"

class IoSelect : public IoEvent {
public:
	IoSelect(std::string name);
	~IoSelect();

	virtual int add_ioEvent(unsigned int fd, unsigned int event, Func func, void* args);
	virtual int add_ioEventRead(unsigned int fd, Func func, void* args);
	virtual int add_ioEventWrite(unsigned int fd, Func func, void *args);
	virtual int add_ioEventAccept(unsigned int fd, Func func, void *args);
	virtual bool is_readEvent(unsigned int event);
	virtual bool is_writeEvent(unsigned int event);
	virtual void del_ioEvent(unsigned int fd);//remove fd event
	virtual void run_loop();
private:
	//run,ms
	void run_loopWithTimeout(int timeout);
private:
	MutexLock mutexLock;
	fd_set fdset;
	unsigned int maxfd;
	std::set<unsigned int, std::greater<unsigned int> > fdsetSet;
};



#endif /* IOMULTIPLEX_IOSELECT_H_ */
