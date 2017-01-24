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
* @ClassName: oneproxyserver.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月29日
*
*/

#ifndef ONEPROXYSERVER_H_
#define ONEPROXYSERVER_H_

#include <iostream>
#include <string>
#include <list>

#include "define.h"
#include "tcpserver.h"
#include "thread.h"
#include "tool.h"

class ConnectManager;
class OneproxyServer:public TcpServer, public Thread{
public:

	OneproxyServer(std::string threadName = std::string("accept_thread"))
		:Thread(thread_type_accept, threadName) {
		this->connectManager = NULL;
		this->stop = false;
		this->startSuccess = false;
	}

	bool get_stop() {
		return this->stop;
	}

	void set_stop() {
		this->stop = true;
	}

	ConnectManager *get_connectManager() {
		return this->connectManager;
	}

	void set_connectManager(ConnectManager* connManager) {
		this->connectManager = connManager;
	}

	void start_success(){this->startSuccess = true;}
	bool get_startSuccess(){return this->startSuccess;}
	static thread_start_func(start);

	virtual void accept_clientRequest(NetworkSocket *clientSocket);
private:
	ConnectManager *connectManager;
	bool stop;
	volatile bool startSuccess;
};

class AcceptThreadManager{
public:
	AcceptThreadManager(){}

	void start(unsigned int threadNum, ConnectManager* connManager,
			std::string serverAddr, std::set<unsigned int>& portList, int listenBackLog = 128) {
		for (unsigned int i = 0; i < threadNum; ++i) {
			OneproxyServer* ops = new OneproxyServer(Tool::args2string("acceptThread:%d", i));
			if (listenBackLog > 0) {
				ops->set_listenBackLog(listenBackLog);
			}
			ops->set_connectManager(connManager);
			ops->set_tcpServer(serverAddr, portList);
			ops->startThread(OneproxyServer::start, ops);
			//wait thread start. for update config online
			/***
			 * sleep的时间不能太短，如果太短当在valgrind检测时，会导致阻塞。
			 * **/
			while(ops->get_startSuccess() == false) {SystemApi::system_sleep(1000);}
			this->acceptThreadList.push_back(ops);
		}
	}
	~AcceptThreadManager() {
		this->stop_thread();
	}

	void stop_thread() {
		std::list<OneproxyServer*>::iterator it = this->acceptThreadList.begin();
		for (; it != this->acceptThreadList.end();) {
			OneproxyServer* ops = *it;
			it = this->acceptThreadList.erase(it);
			if (ops->get_stop() == false) {
				ops->set_stop();
			}
			ops->joinThread();
			delete ops;
		}
	}

	void stop_accept() {
		std::list<OneproxyServer*>::iterator it = this->acceptThreadList.begin();
		for(; it != this->acceptThreadList.end(); ++it) {
			OneproxyServer* ops = *it;
			ops->stop_tcpServer();
		}
	}
private:
	std::list<OneproxyServer*> acceptThreadList;
};
#endif /* ONEPROXYSERVER_H_ */
