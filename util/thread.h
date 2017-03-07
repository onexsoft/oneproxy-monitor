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
* @FileName: thread.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月5日
*
*/

#ifndef THREAD_H_
#define THREAD_H_

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <string>
#include <iostream>

#include "logger.h"
#include "define.h"

#ifdef WIN32
#define thread_func_return_type DWORD WINAPI
#define thread_func_args_type LPVOID
#define thread_id_t HANDLE
#else
#define thread_func_return_type void*
#define thread_func_args_type void*
#define thread_id_t pthread_t
#endif

#define thread_start_func(func_name) thread_func_return_type func_name(thread_func_args_type args)
typedef thread_func_return_type (*ThreadFunc)(thread_func_args_type args);

typedef enum {
	thread_type_init = 0,
	thread_type_client,
	thread_type_manager,
	thread_type_httpserver,
	thread_type_vip,
	thread_type_accept,
	thread_type_work,
	thread_type_db,
} ThreadType;

class Thread{
public:
	Thread(ThreadType type, std::string threadName);
	~Thread();

	void joinThread();
	int startThread(ThreadFunc func, void* args);

	u_uint64 get_threadId();
	ThreadType get_threadType();
	std::string get_threadName();
	void block_threadSignal();
private:
	static thread_start_func(start);
private:
	thread_id_t threadId;
	ThreadType type;
	std::string threadName;

	ThreadFunc userFunc;
	void* userFuncArgs;
};

#endif /* THREAD_H_ */
