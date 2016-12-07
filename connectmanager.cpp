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
OneproxyServer ConnectManager::oneproxyServer;

ConnectManager::ConnectManager(int threadNum):
	httpServer(config()->get_httpServerAddr(), config()->get_httpServerPort(), std::string("httpserver")),
	vipThread(config()->get_vipIfName(), config()->get_vipAddress(), std::string("vipthread")),
	assistThread(this)
{
	ConnectManager::oneproxyServer.set_connectManager(this);

	int i = 0;
	this->stop = false;
	this->threadNum = threadNum;
	for (i = 1; i <= this->threadNum; ++i) {
		ClientThread *ct = new ClientThread(this, Tool::args2string("workThread:%d", i));
		this->threadMap[ct->get_threadId()] = ct;
		this->runningTaskQueue[ct->get_threadId()] = std::map<int, NetworkSocket*>();
	}
}

ConnectManager::~ConnectManager()
{
	for (int i = 0; i < this->threadNum; ++i) {
		delete this->threadMap[i];
		this->threadMap[i] = NULL;
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
	mutexLock.unlock();
	record()->record_acceptClientConn();
}

int ConnectManager::get_taskSize()
{
	return this->taskQueue.size();
}

NetworkSocket* ConnectManager::get_task()
{
	NetworkSocket* ns = NULL;
	mutexLock.lock();
	if (this->taskQueue.size() > 0) {
		ns = this->taskQueue.front();
		this->taskQueue.pop();
		record()->record_startHandingConn();
	}
	mutexLock.unlock();
	return ns;
}

MutexLock* ConnectManager::get_mutexLock()
{
	return &this->mutexLock;
}

void ConnectManager::set_stop()
{
	assistThread.stop();
	httpServer.stop();
	ThreadMapType::iterator it = this->threadMap.begin();
	for (; it != this->threadMap.end(); ++it) {
		ClientThread * ct = (ClientThread*)it->second;
		ct->set_stop();
	}

	ConnectManager::oneproxyServer.set_stop();
}

NetworkSocket* ConnectManager::find_runningTask(u_uint64 threadId, int fd)
{
	NetworkSocket *ns = NULL;

	ThreadTaskQueue::iterator it = this->runningTaskQueue.find(threadId);
	if (it == this->runningTaskQueue.end()) {
		return ns;
	}

	FdNetworkSocketMap::iterator tit = it->second.find(fd);
	if (tit == it->second.end()) {
		return ns;
	}

	ns = tit->second;
	return ns;
}

int ConnectManager::add_runningTask(u_uint64 threadId, NetworkSocket* ns)
{
	NetworkSocket* tns = find_runningTask(threadId, ns->get_fd());
	if (tns != NULL) {
		logs(Logger::ERR, "task(fd:%d) always in thread(%llu) running queue", ns->get_fd(), threadId);
		return -1;
	}
	this->runningTaskQueueMutexLock.lock();
	(this->runningTaskQueue[threadId])[ns->get_fd()] = ns;
	this->runningTaskQueueMutexLock.unlock();

	return 0;
}

void ConnectManager::finished_task(u_uint64 threadId, NetworkSocket *ns)
{
	logs(Logger::INFO, "thread(%lld) finished task fd(%d)", threadId, ns->get_fd());
	ThreadTaskQueue::iterator it = this->runningTaskQueue.find(threadId);
	if (it == this->runningTaskQueue.end()) {
		logs(Logger::ERR, "no thread(%lld) in runningTaskQueue", threadId);
		return;
	}

	FdNetworkSocketMap::iterator tit = it->second.find(ns->get_fd());
	if (tit == it->second.end()) {
		logs(Logger::ERR, "no fd(%d) in thread(%lld) runningTaskQueue", ns->get_fd(), threadId);
		return;
	}
	this->runningTaskQueueMutexLock.lock();
	it->second.erase(tit);
	this->runningTaskQueueMutexLock.unlock();

	logs(Logger::DEBUG, "runningTaskQueue.size: %d", it->second.size());
	record()->record_closeClientConn();
}

u_uint64 ConnectManager::get_clientThread()
{
	ThreadTaskQueue::iterator it = runningTaskQueue.begin();
	u_uint64 resultThread = it->first;
	unsigned int leastTask = it->second.size();
	for (; it != runningTaskQueue.end(); ++it) {
		if (it->second.size() < leastTask) {
			resultThread = it->first;
			leastTask = it->second.size();
		}
	}
	return resultThread;
}

void ConnectManager::alloc_task()
{
	if (get_runningTaskQueueSize() >= config()->get_maxConnectNum())
		return;

	logs(Logger::INFO, "taskQueue: %d, runningTaskQueue.size: %d",
			this->taskQueue.size(), get_runningTaskQueueSize());
	NetworkSocket* ns = this->get_task();
	if (ns == NULL)
		return;

	u_uint64 leastTaskThread = this->get_clientThread();
	ClientThread* ct = (ClientThread*)this->threadMap[leastTaskThread];
	if (ct->add_task(ns)) {//增加任务到客户线程中失败。
		this->add_task(ns);//重新增加到等待队列中。
		return;
	}

	if (this->add_runningTask(leastTaskThread, ns)) {
		logs(Logger::ERR, "add running task error, close fd(%d)", ns->get_fd());
		delete ns;
		return;
	}
}

unsigned int ConnectManager::get_runningTaskQueueSize()
{
	unsigned int size = 0;
	ConnectManager::ThreadTaskQueue::iterator it = this->runningTaskQueue.begin();
	for (; it != this->runningTaskQueue.end(); ++it) {
		size += it->second.size();
	}
	return size;
}

void ConnectManager::start()
{
	SystemApi::system_setThreadName("mainThread");
	ConnectManager::oneproxyServer.set_tcpServer(config()->get_oneproxyAddr(), config()->get_oneproxyPortSet());
	if (ConnectManager::oneproxyServer.create_tcpServer()) {
		logs(Logger::ERR, "create tcp server error");
		this->set_stop();
		return;
	}

	//信号处理函数
	signal(SIGTERM, ConnectManager::handle_signal);
	signal(SIGINT, ConnectManager::handle_signal);
	signal(SIGUSR1, ConnectManager::handle_signal);
	signal(SIGUSR2, ConnectManager::handle_signal);
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif
	signal(SIGFPE, SIG_IGN);

	//close old process.
	if (config()->get_pidFilePath().size() > 0) {
		if (PidManager::handle_reboot(config()->get_pidFilePath().c_str())) {
			logs(Logger::ERR, "reboot error");
			return;
		}
	}

	if (config()->get_pidFilePath().size()) {
		if(PidManager::save_pid(config()->get_pidFilePath().c_str())) {
			logs(Logger::ERR, "write pid error");
			return;
		}
	}

	logs(Logger::ERR, "start to connect  manager...");
	while(!ConnectManager::stop || (this->get_taskSize() > 0)) {
		if (this->get_taskSize() > 0) {
			this->alloc_task();
			ConnectManager::oneproxyServer.run_server(0);
		} else {
			if (ConnectManager::oneproxyServer.get_stop()) {
				SystemApi::system_sleep(500);
			} else {
				ConnectManager::oneproxyServer.run_server(500);
			}
		}
	}

	//wait running conneciton finished.
	while(this->get_runningTaskQueueSize()) {
		SystemApi::system_sleep(500);
	}

	this->set_stop();

	//6. unlink pid file
	if (config()->get_pidFilePath().size()) {
		PidManager::unlink_pid(config()->get_pidFilePath().c_str());
	}
}

void ConnectManager::handle_signal(int sig)
{
	logs(Logger::ERR, "start to logout...");
	ConnectManager::oneproxyServer.stop_tcpServer();
	ConnectManager::stop = true;
	return ;
}
