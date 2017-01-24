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
* @FileName: clientthread.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月4日
*
*/


#ifndef CLIENTTHREAD_H_
#define CLIENTTHREAD_H_

#include <queue>
#include "thread.h"
#include "networksocket.h"
#include "record.h"
#include "ioevent.h"
#include "connection.h"
#include "mutexlock.h"
#include "uspinlock.h"

typedef struct _try_failed_connection_t{
	void* connection;
	void* clientThread;
}TryFailedConnection;

typedef struct _client_task_stat_t{
	unsigned int _wait_connection;
	unsigned int _doing_connection;
	unsigned int _fail_connection;
	//USpinLock m_lock;
	_client_task_stat_t() {
		this->_wait_connection = 0;
		this->_doing_connection = 0;
		this->_fail_connection = 0;
	}
	void recv_connection() {
		//m_lock.lock();
		this->_wait_connection++;
		//m_lock.unlock();
	}
	void close_connection() {
		this->_doing_connection--;
	}
	void doing_connection() {
		//m_lock.lock();
		this->_wait_connection--;
		//m_lock.unlock();
		this->_doing_connection++;
	}
	void done_connection() {
		this->_doing_connection --;
	}
	void doing_failConnection() {
		this->_doing_connection --;
		this->_fail_connection++;
	}
	void retry_failConnection() {
		this->_fail_connection--;
		this->_doing_connection++;
	}
	unsigned int sum_connection() {
		return this->_wait_connection
				+ this->_doing_connection
				+ this->_fail_connection;
	}
}ClientTaskStat;

class ConnectManager;
class ClientThread : public Thread {
public:
	ClientThread(ConnectManager* connManager, std::string threadName);
	~ClientThread();
	void set_stop();
	bool get_stop();
	unsigned int get_threadTaskNum();
	int get_socketPairReadFd();
	int get_socketPairWriteFd();
	int write_socketPair(const void* data, const unsigned int dataLen);

private:
	//check connection is timeout or not
	void check_connectionTimeout();
	unsigned int get_ConnectionNum();
	int add_task(NetworkSocket* ns);
	void handle_readFrontData(Connection& con);
	void finished_connection(Connection* con, ConnFinishType type);
	void get_serverFailed(Connection *con);
	int alloc_server(Connection* con);
	int parse_frontDataPacket(Connection* con);
	//从数据库组中分配数据库
	int get_databaseFromGroup(Connection& con);

	void handle_readBackendData(Connection& con);
	int parse_backendDataPacket(Connection* con);

	int send_data(Connection& conn, bool sendToClient = true);
	//当isFront为true时，表示向前端套接字写数据，否则表示向后端套接字写数据
	int write_data(Connection& con, bool isFront);
	void add_FailConnQueue(Connection* conn);

	static thread_start_func(start);
	static void rw_frontData(unsigned int fd, unsigned int events, void* args);
	static void rw_backendData(unsigned int fd, unsigned int events, void* args);
	static void read_clientSocket(unsigned int fd, unsigned int events, void* args);
	static void check_connectionTimeout(void* args);
	static void retry_failedConnection(void* args);
	static void check_quit(void* args);

private:
	declare_type_alias(ConnectionList, std::list<Connection*>)
	IoEvent *ioEvent;
	ConnectManager *connManager;
	ConnectionList connectList;
	MutexLock clientLock;
	bool stop;

	std::queue<Connection*> FailConnQueue;
	ClientTaskStat taskStat;
	int spfd[2];
};

#endif /* CLIENTTHREAD_H_ */
