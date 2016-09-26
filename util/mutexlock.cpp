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
* @FileName: mutexlock.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月5日
*
*/
#include "record.h"
#include "mutexlock.h"

#include <sys/time.h>

MutexLock::MutexLock()
{
	this->record = NULL;
	initMutex();
}

MutexLock::MutexLock(std::string name)
{
	this->mutexName = name;
	this->record = NULL;
	initMutex();
}

MutexLock::MutexLock(std::string name, Record *record)
{
	this->mutexName = name;
	this->record = record;
	initMutex();
}

MutexLock::~MutexLock()
{
	this->mutexName.clear();
#ifdef WIN32
	::CloseHandle(this->mutex);
	::CloseHandle(this->mutexCond);
#else
	pthread_mutex_destroy(&this->mutex);
	pthread_cond_destroy(&this->mutexCond);
#endif
}

void MutexLock::lock()
{
#ifdef WIN32
	::WaitForSingleObject(this->mutex, INFINITE);
#else
	pthread_mutex_lock(&this->mutex);
#endif

	if (this->record) {
		record->record_lock(this);
	}
}

void MutexLock::unlock()
{
#ifdef WIN32
	::ReleaseMutex(this->mutex);
#else
	pthread_mutex_unlock(&this->mutex);
#endif

	if (this->record)
		this->record->record_unlock(this);
}

void MutexLock::set_name(std::string name)
{
	this->mutexName = name;
}

std::string MutexLock::get_name()
{
	return this->mutexName;
}

void MutexLock::wait_mutexCond(int timeout)
{

#ifdef WIN32
	if (timeout < 0) {
		::SignalObjectAndWait(this->mutex, this->mutexCond, INFINITE, FALSE);
	} else {
		::SignalObjectAndWait(this->mutex, this->mutexCond, timeout, FALSE);
	}
	this->lock();
#else
	if (timeout < 0){
		pthread_cond_wait(&this->mutexCond, &this->mutex);
	} else {
		struct timespec t;
		struct timeval now;
		gettimeofday(&now, NULL);
		t.tv_sec  = now.tv_sec;
		t.tv_nsec = now.tv_usec * 1000 + timeout * 1000000;
		pthread_cond_timedwait(&this->mutexCond, &this->mutex, &t);
	}
#endif

}

void MutexLock::signal_mutexCond()
{
#ifdef WIN32
	::PulseEvent(this->mutexCond);
#else
	pthread_cond_signal(&this->mutexCond);
#endif
}

void MutexLock::set_record(Record* record)
{
	this->record = record;
}
