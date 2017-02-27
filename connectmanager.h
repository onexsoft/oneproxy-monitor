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
* @FileName: connectmanager.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月4日
*
*/

#ifndef CONNECTMANAGER_H_
#define CONNECTMANAGER_H_


#include "vip.h"
#include "record.h"
#include "thread.h"
#include "httpserver.h"
#include "mutexlock.h"
#include "networksocket.h"
#include "assistthread.h"
#include "oneproxyserver.h"
#include "clientthread.h"

#include <map>
#include <queue>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include "managerbase.h"

class ConnectManager: public ManagerBase
{
public:
	ConnectManager(int threadNum = 5);
	~ConnectManager();
	void add_task(NetworkSocket *clientSocket);
	MutexLock* get_mutexLock();
	ClientThread* get_minTaskThread();
	int get_taskSize();
protected:
	virtual void start_child();

private:
	unsigned int get_allThreadTaskSize();
	void alloc_task();//分配任务
	static void handle_signal(int sig);

private:
	declare_type_alias(ThreadMapType, std::map<u_uint64, Thread*>)//key: threadId
	declare_type_alias(TaskQueue, std::queue <NetworkSocket* >)

	int threadNum;
	static bool stop;
	ThreadMapType threadMap; //key: threadId
	TaskQueue taskQueue; //need to handle sock
	static MutexLock mutexLock;
	static AcceptThreadManager acceptThreadManager;
};
#endif /* CONNECTMANAGER_H_ */
