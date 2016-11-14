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

#include "thread.h"
#include "networksocket.h"
#include "record.h"
#include "ioevent.h"
#include "connection.h"
#include "mutexlock.h"

class ConnectManager;
class ClientThread : public Thread {
public:
	ClientThread(ConnectManager* connManager, std::string threadName);
	~ClientThread();

	void set_stop();
	int add_task(NetworkSocket* ns);

private:
	void handle_readFrontData(unsigned int fd);
	void finished_connection(Connection* con);
	void get_serverFailed(Connection *con);
	int alloc_server(Connection* con);
	int parse_frontDataPacket(Connection* con);
	//从数据库组中分配数据库
	int get_databaseFromGroup(Connection& con);

	void handle_readBackendData(unsigned int fd);
	int parse_backendDataPacket(Connection* con);

	int send_data(Connection& conn, bool sendToClient = true);
	//当isFront为true时，表示向前端套接字写数据，否则表示向后端套接字写数据
	void write_data(Connection& con, bool isFront);
	Connection* get_connection(unsigned int fd);
	void remove_connectFdRelation(unsigned int fd);

	static thread_start_func(start);
	static void rw_frontData(unsigned int fd, unsigned int events, void* args);
	static void rw_backendData(unsigned int fd, unsigned int events, void* args);

private:
	declare_type_alias(ConnectionTypeMap, std::map<unsigned int, Connection*>)
	IoEvent *ioEvent;
	ConnectManager *connManager;
	ConnectionTypeMap connectTypeMap;
	MutexLock clientLock;
};

#endif /* CLIENTTHREAD_H_ */
