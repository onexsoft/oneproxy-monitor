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
* @FileName: connectmanager.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月4日
*
*/

#include "time.h"
#include "tool.h"
#include "record.h"
#include "config.h"
#include "logger.h"
#include "clientthread.h"
#include "connectmanager.h"
#include "pidmanager.h"

#include <signal.h>
#include <unistd.h>

bool ConnectManager::stop = false;
AcceptThreadManager ConnectManager::acceptThreadManager;
MutexLock ConnectManager::mutexLock;

ConnectManager::ConnectManager(int threadNum)
{
	int i = 0;
	this->stop = false;
	this->threadNum = threadNum;
	for (i = 1; i <= this->threadNum; ++i) {
		ClientThread *ct = new ClientThread(this, Tool::args2string("workThread:%d", i));
		this->threadMap[ct->get_threadId()] = ct;
	}
	acceptThreadManager.start(config()->get_acceptThreadNum(), this,
			config()->get_oneproxyAddr(), config()->get_oneproxyPortSet(),
			config()->get_listenBackLog());

	this->set_signalHandleFunc(&ConnectManager::handle_signal);
}

ConnectManager::~ConnectManager()
{
	ThreadMapType::iterator it = this->threadMap.begin();
	for (; it != this->threadMap.end(); ++it) {
		ClientThread* ct = (ClientThread*)((*it).second);
		if (ct->get_stop() == false) {
			ct->set_stop();
		}
		delete ct;
	}

	while(taskQueue.size() > 0) {
		NetworkSocket * ns = taskQueue.front();
		taskQueue.pop();
		delete ns;
	}
}

void ConnectManager::add_task(NetworkSocket* clientSocket)
{
	logs(Logger::DEBUG, "add_task: %d", clientSocket->get_fd());
	mutexLock.lock();
	this->taskQueue.push(clientSocket);
	mutexLock.signal_mutexCond();
	mutexLock.unlock();
	record()->record_clientConn2GlobalQueue();
}

MutexLock* ConnectManager::get_mutexLock()
{
	return &mutexLock;
}

ClientThread* ConnectManager::get_minTaskThread()
{
	ClientThread* thread = NULL;
	unsigned int minTask = -1;
	unsigned int sumTask = 0;
	ThreadMapType::iterator it = this->threadMap.begin();
	for (; it != this->threadMap.end(); ++it) {
		ClientThread* ct = (ClientThread*)((*it).second);
		sumTask += ct->get_threadTaskNum();
		if (minTask == (unsigned int)-1) {
			thread = ct;
			minTask = ct->get_threadTaskNum();
		} else if (ct->get_threadTaskNum() < minTask) {
			thread = ct;
			minTask = ct->get_threadTaskNum();
		}
	}

	if (sumTask > config()->get_maxConnectNum()) {
		return NULL;
	}
	return thread;
}

int ConnectManager::get_taskSize()
{
	return this->taskQueue.size();
}

unsigned int ConnectManager::get_allThreadTaskSize()
{
	unsigned int sumTask = 0;
	ThreadMapType::iterator it = this->threadMap.begin();
	for (; it != this->threadMap.end(); ++it) {
		ClientThread* ct = (ClientThread*)((*it).second);
		sumTask += ct->get_threadTaskNum();
	}
	return sumTask;
}

void ConnectManager::alloc_task()
{
	if (this->get_taskSize() <= 0)
		return;

	unsigned int numPerThread = (unsigned int)(config()->get_maxConnectNum() / this->threadMap.size());
	ThreadMapType::iterator it = this->threadMap.begin();
	for (; it != this->threadMap.end(); ++it) {
		ClientThread* ct = (ClientThread*)((*it).second);
		while(ct->get_threadTaskNum() < numPerThread) {
			if (this->get_taskSize() <= 0)
				return;

			mutexLock.lock();
			if (this->get_taskSize() > 0) {
				NetworkSocket* ns = this->taskQueue.front();
				this->taskQueue.pop();
				if (ct->write_socketPair(ns, sizeof(ns))) {
					logs(Logger::ERR, "add client socket to thread error");
					delete ns;
				}
				record()->record_outGlobalQueue();
			}
			mutexLock.unlock();
		}
	}

	while(this->taskQueue.size()) {
		ClientThread* ct = this->get_minTaskThread();
		if (ct == NULL)
			return;

		NetworkSocket * ns = NULL;
		mutexLock.lock();
		if (this->taskQueue.size() > 0) {
			ns = this->taskQueue.front();
			this->taskQueue.pop();
		}
		mutexLock.unlock();

		if (ns != NULL) {
			if (ct->write_socketPair(ns, sizeof(ns))) {
				logs(Logger::ERR, "write client socket to thread error");
				delete ns;
			}
			record()->record_outGlobalQueue();
		}
	}
}

void ConnectManager::start_child()
{
	logs(Logger::ERR, "start to connect  manager...");
	while(!ConnectManager::stop || (this->get_taskSize() > 0)) {
		if (this->get_taskSize() > 0
				&& this->get_allThreadTaskSize() < config()->get_maxConnectNum()) {
			this->alloc_task();
		} else {
			mutexLock.lock();
			mutexLock.wait_mutexCond();
			mutexLock.unlock();
		}
	}
}

void ConnectManager::handle_signal(int sig)
{
	logs(Logger::ERR, "start to logout...");
	ConnectManager::stop = true;
	acceptThreadManager.stop_accept();
	mutexLock.lock();
	mutexLock.signal_mutexCond();
	mutexLock.unlock();
	return ;
}
