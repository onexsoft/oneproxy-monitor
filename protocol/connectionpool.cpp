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
 * @FileName: ConnectionPool.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年11月8日 下午3:51:48
 *  
 */

#include "connectionpool.h"

ConnectionPool::ConnectionPool()
	: mutexLock("connectionPoolLock")
{
	this->idle_timeout_check = 1 * 1000;
	this->idle_timeout_release = 60 * 1000;
	this->current_time = config()->get_globalMillisecondTime();
	this->check_active = false;
}

ConnectionPool::~ConnectionPool()
{
	std::map<ConnPoolKey, SockInfoDeque>::iterator it = this->idle_sock.begin();
	for (; it != this->idle_sock.end(); ++it) {
		SockInfoDeque& sid = it->second;
		while(sid.size() > 0) {
			SockInfo* si = sid.front();
			sid.pop_front();
			if (si != NULL) {
				delete si;
				si = NULL;
			}
		}
	}
}

ConnectionPool& ConnectionPool::get_pool() {
	static ConnectionPool pool;
	return pool;
}

void ConnectionPool::set_checkActive()
{
	this->check_active = true;
}

NetworkSocket* ConnectionPool::get_backendConnect(std::string dbAddr, int port, std::string connArgs)
{
	NetworkSocket* ns = NULL;
	logs(Logger::DEBUG, "get backend connecton from pool");
	mutexLock.lock();
	do{
		if (this->idle_sock.size() <= 0)
			break;

		ConnPoolKey key;
		key.dbAddr = dbAddr;
		key.port = port;
		key.otherIdentify = connArgs;
		key.keyHashCode = key.gen_keyHashCode();
		std::map<ConnPoolKey, SockInfoDeque>::iterator it = this->idle_sock.find(key);
		if (it == this->idle_sock.end()) {
			break;
		}

		SockInfoDeque& sis = it->second;
		if (sis.size() <= 0)
			break;

		SockInfo* si = sis.back();
		sis.pop_back();
		ns = si->socket;
		if (ns != NULL) {
			this->used_sock[ns->get_fd()] = si;
			si->start_use_time = this->current_time;
		}
	}while(0);
	mutexLock.unlock();

	if (ns != NULL)
		logs(Logger::DEBUG, "from pool get fd: %d", ns->get_fd());
	return ns;
}

int ConnectionPool::set_backendConnect(NetworkSocket* socket)
{
	logs(Logger::DEBUG, "restore fd: %d to pool", socket->get_fd());
	if (this->used_sock.size() <= 0) {
		logs(Logger::ERR, "used_sock is zero, error");
		return -1;
	}

	mutexLock.lock();
	std::map<unsigned int, SockInfo*>::iterator it = this->used_sock.find(socket->get_fd());
	if (it == this->used_sock.end()) {
		logs(Logger::ERR, "not find fd(%d) in used_sock(size: %d)",
				socket->get_fd(), this->used_sock.size());
		mutexLock.unlock();
		return -1;
	}

	ConnPoolKey key;
	key.dbAddr = socket->get_address();
	key.port = socket->get_port();
	key.otherIdentify = socket->connArgsMap2String();
	key.keyHashCode = key.gen_keyHashCode();
	SockInfoDeque& sid =  this->idle_sock[key];
	it->second->start_use_time = this->current_time; //更新被使用的时间
	sid.push_back(it->second);
	this->used_sock.erase(socket->get_fd());
	mutexLock.unlock();
	return 0;
}

void ConnectionPool::save_backendConnect(NetworkSocket* socket, FreeFunc free_socket,
		CheckSockActiveFunc active_func, bool is_using)
{
	logs(Logger::DEBUG, "save backend connection to pool fd: %d", socket->get_fd());
	SockInfo* sockInfo = new SockInfo();
	sockInfo->active_func = active_func;
	sockInfo->free_socket = free_socket;
	sockInfo->socket = socket;

	mutexLock.lock();
	if (is_using) {
		this->used_sock[socket->get_fd()] = sockInfo;
		sockInfo->start_use_time = this->current_time;
	} else {
		ConnPoolKey key;
		key.dbAddr = socket->get_address();
		key.port = socket->get_port();
		key.otherIdentify = socket->connArgsMap2String();
		key.keyHashCode = key.gen_keyHashCode();
		this->idle_sock[key].push_front(sockInfo);
	}
	mutexLock.unlock();
}

void ConnectionPool::release_backendSocket(NetworkSocket* socket) {

	if (this->used_sock.size() <= 0)
		return;

	mutexLock.lock();
	do{
		std::map<unsigned int, SockInfo*>::iterator it = this->used_sock.find(socket->get_fd());
		if (it == this->used_sock.end()) {
			break;
		}
		delete it->second;
		this->used_sock.erase(it);
	}while(0);
	mutexLock.unlock();
}

void ConnectionPool::check_connectActive() {
	this->current_time = config()->get_globalMillisecondTime();
	mutexLock.lock();
	std::map<ConnPoolKey, SockInfoDeque>::iterator it = this->idle_sock.begin();
	for (; it != this->idle_sock.end(); ++it) {
		SockInfoDeque& sid = it->second;
		if (sid.size() <= 0) {
			continue;
		}

		SockInfoDeque::iterator rit = sid.begin();
		for (; rit != sid.end(); ) {
			SockInfo& si = *(*rit);
			if (this->current_time - si.start_use_time <= this->idle_timeout_check) {
				rit = rit + 1;
				break;//说明后面的socket都是最近使用过的
			}

			if (this->current_time - si.start_use_time > this->idle_timeout_release) {
				logs(Logger::DEBUG, "delete fd: %d", si.socket->get_fd());
				delete (*rit);
				rit = sid.erase(rit);
				continue;
			}

			if (si.active_func == NULL || this->check_active == false) {
				++rit;
				continue;
			}

			if (false == ((*rit)->check_active())) {
				delete (*rit);
				rit = sid.erase(rit);
			} else {
				++rit;
			}
		}
	}
	mutexLock.unlock();
	this->current_time = config()->get_globalMillisecondTime();
}

void ConnectionPool::set_idleTimeoutCheck(int second)
{
	this->idle_timeout_check = second * 1000;
}

void ConnectionPool::set_idleTimeoutRelease(int second)
{
	this->idle_timeout_release = second * 1000;
}
