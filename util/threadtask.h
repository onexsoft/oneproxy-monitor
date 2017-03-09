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
 * @FileName: threadtask.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年3月2日 下午1:49:42
 *  
 */

#ifndef UTIL_THREADTASK_H_
#define UTIL_THREADTASK_H_

#include <list>
#include <iostream>

#include "thread.h"
#include "mutexlock.h"

typedef void (*TaskHandleFunc) (void* data, void* args);

template<class T>
class ThreadTask : public Thread{
public:
	ThreadTask(ThreadType threadType, std::string threadName, TaskHandleFunc func, void* func_args);
	virtual ~ThreadTask(){}
	void add_taskData(T data) {
		this->m_lock.lock();
		this->m_dataList.push_back(data);
		this->m_lock.signal_mutexCond();
		this->m_lock.unlock();
	}
	void handle_taskData();
	void set_stop(){
		this->m_isStop = true;
		this->m_lock.lock();
		this->m_lock.signal_mutexCond();
		this->m_lock.unlock();
	}
private:
	static thread_start_func(start);
private:
	typedef std::list<T> TypeList;
	TypeList m_dataList;
	TaskHandleFunc m_func;
	void* m_funcArgs;
	bool m_isStop;
	MutexLock m_lock;
};

template<class T>
ThreadTask<T>::ThreadTask(ThreadType threadType, std::string threadName, TaskHandleFunc func, void* func_args)
		: Thread(threadType, threadName) {
	// TODO Auto-generated constructor stub
	this->m_func = func;
	this->m_funcArgs = func_args;
	this->m_isStop = false;
	this->startThread(ThreadTask<T>::start, this);
}

template<class T>
void ThreadTask<T>::handle_taskData() {
	T task;
	this->m_lock.lock();
	if (this->m_dataList.size()) {
		task = this->m_dataList.front();
		this->m_dataList.pop_front();
		this->m_lock.unlock();
	} else {
		this->m_lock.unlock();
		return;
	}
	this->m_func((void*)&task, this->m_funcArgs);
	return;
}

template<class T>
thread_start_func(ThreadTask<T>::start) {
	ThreadTask* tt = (ThreadTask*)args;
	while(tt->m_isStop == false || tt->m_dataList.size() > 0) {
		if (tt->m_dataList.size() > 0) {
			tt->handle_taskData();
		} else {
			tt->m_lock.lock();
			tt->m_lock.wait_mutexCond();
			tt->m_lock.unlock();
		}
	}
	return 0;
}
#endif /* UTIL_THREADTASK_H_ */
