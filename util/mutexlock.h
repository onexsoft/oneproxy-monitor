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
* @FileName: mutexlock.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月5日
*
*/

#ifndef UTIL_MUTEXLOCK_H_
#define UTIL_MUTEXLOCK_H_

#include <string>
#include <iostream>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

#ifdef WIN32
typedef HANDLE MutexLockHandle;
typedef HANDLE MutexCondHandle;
#else
typedef pthread_mutex_t MutexLockHandle;
typedef pthread_cond_t MutexCondHandle;
#endif

class Record;
class MutexLock{
public:
	MutexLock();
	MutexLock(std::string name);
	MutexLock(std::string name, Record *record);
	~MutexLock();

	inline void initMutex();
	void lock();
	void unlock();
	void set_name(std::string name);
	std::string get_name();
	void wait_mutexCond(int timeout = -1);//单位为毫秒
	void signal_mutexCond();

	void set_record(Record *record);
private:
	MutexLockHandle mutex;
	MutexCondHandle mutexCond;
	std::string mutexName;
	Record *record;
};
void MutexLock::initMutex()
{
#ifdef WIN32
	this->mutex = ::CreateMutex(NULL, false, NULL);
	this->mutexCond = ::CreateEvent(NULL, false, false, "win mutex condition");
#else
	pthread_mutex_init(&this->mutex, NULL);
	pthread_cond_init(&this->mutexCond, NULL);
#endif
}

#endif /* UTIL_MUTEXLOCK_H_ */
