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
* @FileName: thread.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月5日
*
*/

#include "thread.h"
#include "systemapi.h"

//Thread::Thread()
//{
//	this->threadId = 0;
//	this->type = thread_type_init;
//}

Thread::Thread(ThreadType type, std::string tname)
	:threadName(tname)
{
	this->type = type;
	this->threadId = 0;
	this->userFunc = NULL;
	this->userFuncArgs = NULL;
}

Thread::~Thread()
{
	if (this->threadId != 0) {
#ifdef WIN32
		WaitForSingleObject(this->threadId, INFINITE);
#else
		pthread_join(this->threadId, NULL);
#endif
	}
}

int Thread::startThread(ThreadFunc func, void*args) {
	this->userFunc = func;
	this->userFuncArgs = args;
#ifdef WIN32
	this->threadId = ::CreateThread(NULL, 0, Thread::start, this, 0, NULL);
#else
	if (pthread_create(&this->threadId, NULL, Thread::start, this) < 0) {
		logs(Logger::ERR, "create pthread error");
		return -1;
	}
#endif
	return 0;
}

u_uint64 Thread::get_threadId()
{
	return (u_uint64)(this->threadId);
}

ThreadType Thread::get_threadType()
{
	return this->type;
}

std::string Thread::get_threadName()
{
	return this->threadName;
}

thread_start_func(Thread::start)
{
	Thread* th = (Thread*)args;

	//1. set username
	SystemApi::system_setThreadName(th->threadName);

	return th->userFunc(th->userFuncArgs);
}
