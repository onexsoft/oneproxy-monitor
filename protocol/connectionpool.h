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
 * @FileName: ConnectionPool.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年11月8日 下午3:51:48
 *  
 */

#ifndef PROTOCOL_CONNECTIONPOOL_H_
#define PROTOCOL_CONNECTIONPOOL_H_

#include <map>
#include <deque>
#include <iostream>
#include "mutexlock.h"
#include "systemapi.h"
#include "networksocket.h"
#include "protocolbase.h"

typedef bool (*CheckSockActiveFunc) (NetworkSocket*);
typedef struct _sock_info_t{
	NetworkSocket* socket;
	FreeFunc free_socket;//baseObject中的free_socket函数指针
	CheckSockActiveFunc active_func;//baseObject中的active_func函数指针
	u_uint64 start_use_time;
	_sock_info_t() {
		this->socket = NULL;
		this->free_socket = NULL;
		this->active_func = NULL;
		this->start_use_time = 0;
	}
	~_sock_info_t() {
		logs(Logger::DEBUG, "free fd: %d", this->socket->get_fd());
		if (socket != NULL && free_socket != NULL) {
			(*free_socket)(this->socket);
		}
	}

	bool check_active() {
		if (socket != NULL && active_func != NULL) {
			return (*active_func)(this->socket);
		}
		return false;
	}
} SockInfo;

typedef struct _conn_pool_key_t{
	std::string dbAddr;
	int port;
	bool operator< (const _conn_pool_key_t& a) const {
		if ((strncmp_s(dbAddr, a.dbAddr) < 0) && this->port <= a.port) {
			return true;
		}
		return this->port < a.port;
	}
}ConnPoolKey;

typedef std::deque<SockInfo*> SockInfoDeque;

class ConnectionPool {
private:
	ConnectionPool();
	~ConnectionPool();

public:
	static ConnectionPool& get_pool();

	void set_checkActive();
	//根据后端连接
	NetworkSocket* get_backendConnect(std::string dbAddr, int port);

	//把分配后的连接规划到连接池中
	int set_backendConnect(NetworkSocket* socket);

	//把新创建的连接存储到连接池中
	//is_using为true表示当前socket正在被使用，否则表示当前socket空闲及没有客户端使用此链接
	void save_backendConnect(NetworkSocket* socket, FreeFunc free_socket,
			CheckSockActiveFunc active_func, bool is_using);

	//释放指定连接,只能是否正在使用的连接，空闲的连接不能被外部释放。
	void release_backendSocket(NetworkSocket* socket);

	//检测连接是否有效，如果无效则释放连接
	void check_connectActive();

	//设置检查时间和释放时间
	void set_idleTimeoutCheck(int second);
	void set_idleTimeoutRelease(int second);
private:
	std::map<ConnPoolKey, SockInfoDeque> idle_sock;//空闲的socket
	std::map<unsigned int, SockInfo*> used_sock; //正在使用的socket
	MutexLock mutexLock;
	u_uint64 idle_timeout_check; //空闲多长时间没有被使用，需要进行检测是否active
	u_uint64 idle_timeout_release; //空闲多长时间没有被使用，则需要释放当前连接
	u_uint64 current_time; //用来记录当前的时间，免得每次去读取时间
	bool check_active;//是否需要进行active检测
};

#endif /* PROTOCOL_CONNECTIONPOOL_H_ */
