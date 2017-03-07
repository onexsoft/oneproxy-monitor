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
 * @FileName: parsedata.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月16日 下午4:16:08
 *  
 */

#include "parsedata.h"
#include "protocolbase.h"

ParseData::ParseData()
	: Thread(thread_type_work, "parseData"){
	// TODO Auto-generated constructor stub
	this->m_isStop = false;
	regester_protocol();
	this->startThread(ParseData::start, this);
}

ParseData::~ParseData() {
	// TODO Auto-generated destructor stub
	while(this->m_taskDataList.size()) {
		TaskDataT* td = this->get_taskData();
		if (td) {
			delete td;
			td = NULL;
		}
	}

	TaskDataKeyConnMap::iterator it = taskDataConnMap.begin();
	for(; it != taskDataConnMap.end(); ++it) {
		Connection* conn = it->second;
		if (conn) {
			free_connection(conn);
		}
		it->second = NULL;
	}
}

void ParseData::regester_protocol() {
	std::map<int, std::string>& monitorPortClass = config()->get_monitorPortClassMap();
	std::map<int, std::string>::iterator it = monitorPortClass.begin();
	for(; it != monitorPortClass.end(); ++it) {
		this->add_portHandle(it->first, it->second, NULL, NULL);
	}
}

void ParseData::add_taskData(TaskDataT* taskData)
{
	this->m_lock.lock();
	this->m_taskDataList.push_back(taskData);
	this->m_lock.signal_mutexCond();
	this->m_lock.unlock();
}

void ParseData::add_portHandle(int port, std::string className, MonitorFunc func, void* func_args) {
	this->portHandleMap[port].func = func;
	this->portHandleMap[port].func_args = func_args;
	this->portHandleMap[port].className = className;
}

void ParseData::add_nameHandle(std::string name, std::string className, MonitorFunc func, void* func_args) {
	this->nameHandleMap[name].func = func;
	this->nameHandleMap[name].func_args = func_args;
	this->nameHandleMap[name].className = className;
}

void ParseData::set_stop() {
	this->m_isStop = true;
	this->m_lock.lock();
	this->m_lock.signal_mutexCond();
	this->m_lock.unlock();
}

thread_start_func(ParseData::start) {
	ParseData* pd = (ParseData*)args;

	while(pd->m_isStop == false || pd->m_taskDataList.size() > 0) {
		if (pd->m_taskDataList.size() > 0) {
			pd->handle_taskData(pd->get_taskData());
		} else {
			pd->m_lock.lock();
			pd->m_lock.wait_mutexCond();
			pd->m_lock.unlock();
		}
	}

	return 0;
}

TaskDataT* ParseData::get_taskData() {
	TaskDataT* pd = NULL;
	this->m_lock.lock();
	if (this->m_taskDataList.size()) {
		pd = this->m_taskDataList.front();
		this->m_taskDataList.pop_front();
	}
	this->m_lock.unlock();
	return pd;
}

void ParseData::handle_taskData(TaskDataT* taskData) {
	assert(taskData);
	this->handle_protocolData(taskData);

	delete taskData;
	taskData = NULL;
}

void ParseData::handle_protocolData(TaskDataT* taskData) {
	assert(taskData);

	Connection* conn = NULL;
	TaskDataT* tdt = (TaskDataT*)taskData;
	if (tdt->pi.port == 0) {//no port, no handler.
		return;
	}

	TaskDataKeyConnMap::iterator it = taskDataConnMap.find(tdt->key);
	if ((tdt->flag == CONN_TCP_FIN) && (tdt->data.length() <= 0)) {//conn closed.
		if (it != taskDataConnMap.end()) {
			Connection* conn = it->second;
			taskDataConnMap.erase(it);
			free_connection(conn);
			record()->record_closeClientConn();
		}
		return;
	}
	if ((tdt->flag == CONN_TCP_SYN) && (tdt->data.length() <= 0)) {
		return;//no data
	}

	//get connection and save data.
	if (it == taskDataConnMap.end()) {
		//first packet.
		conn = this->new_connection(tdt);
	} else {
		conn = it->second;
	}
	StringBuf* sb = NULL;
	if (tdt->key.desPort == tdt->pi.port) {//recv data.
		sb = &conn->sock.curclins->get_recvData();
	} else {//send data.
		sb = &conn->sock.curservs->get_recvData();
	}
	if (sb) {
		sb->append(tdt->data.addr(), tdt->data.length());
	}

	//get handle class object.
	if (conn->protocolBase == NULL) {
		conn->protocolBase = (ProtocolBase*)get_protocolBase(tdt);
		if (conn->protocolBase != NULL) {
			conn->protocolBase->protocol_init(*conn);
		} else {
			//logs(Logger::ERR, "protocolBase error");
			free_connection(conn);
			return;
		}
	}

	//save connection when in tcp.
	if (tdt->flag != CONN_UDP) {
		taskDataConnMap[tdt->key] = conn;
	}

	//handle data.
	if (tdt->key.desPort == tdt->pi.port) {//client data packet
		conn->protocolBase->protocol_front(*conn);
	} else {//server data packet.
		conn->protocolBase->protocol_backend(*conn);
	}

	//close connection
	if (tdt->flag == CONN_TCP_FIN) {
		taskDataConnMap.erase(tdt->key);
	}
	if (tdt->flag == CONN_UDP || tdt->flag == CONN_TCP_FIN) {
		free_connection(conn);
		record()->record_closeClientConn();
	}
}

void* ParseData::get_protocolBase(TaskDataT* taskData) {
	assert(taskData);

	std::string className;
	do {
		if (taskData->pi.port > 0) {
			IntFuncParamMap::iterator it = this->portHandleMap.find(taskData->pi.port);
			if (it != this->portHandleMap.end()) {
				if (!it->second.className.empty()) {
					className = it->second.className;
					break;
				}
			}
		}

		if (!taskData->pi.name.empty()) {
			StringFuncParamMap::iterator it = this->nameHandleMap.find(taskData->pi.name);
			if (it != this->nameHandleMap.end()) {
				if (!it->second.className.empty()) {
					className = it->second.className;
					break;
				}
			}
		}

		//use default handle.
		IntFuncParamMap::iterator it = this->portHandleMap.find(-1);
		if (it != this->portHandleMap.end()) {
			if (!it->second.className.empty()) {
				className = it->second.className;
				break;
			}
		}
	}while(0);

	if (className.empty()) {
		return NULL;
	} else {
		ProtocolBase* base = (ProtocolBase*)ProtocolFactory::sharedClassFactory().getClassByName(className);
		return base;
	}
}

Connection* ParseData::new_connection(TaskDataT* tdt)
{
	//first packet.
	Connection* conn = new Connection();
	if (tdt->key.desPort == tdt->pi.port) {
		if (conn->sock.curclins == NULL) {
			conn->sock.curclins = new NetworkSocket();
			conn->sock.curclins->set_address(tdt->key.srcIp);
			conn->sock.curclins->set_port(tdt->key.srcPort);
		}
		if (conn->sock.curservs == NULL) {
			conn->sock.curservs = new NetworkSocket();
			conn->sock.curservs->set_address(tdt->key.desIp);
			conn->sock.curservs->set_port(tdt->key.desPort);
		}
	} else {
		if (conn->sock.curclins == NULL) {
			conn->sock.curclins = new NetworkSocket();
			conn->sock.curclins->set_address(tdt->key.desIp);
			conn->sock.curclins->set_port(tdt->key.desPort);
		}
		if (conn->sock.curservs == NULL) {
			conn->sock.curservs = new NetworkSocket();
			conn->sock.curservs->set_address(tdt->key.srcIp);
			conn->sock.curservs->set_port(tdt->key.srcPort);
		}
	}
	unsigned int clientHashCode = Tool::quick_hash_code(conn->sock.curclins->get_address().c_str(),
			conn->sock.curclins->get_address().length());
	conn->sock.curclins->set_addressHashCode(clientHashCode);
	conn->createConnTime = SystemApi::system_millisecond();
	conn->connection_hashcode = Tool::quick_conn_hash_code(conn->sock.curclins->get_address(), conn->sock.curclins->get_port(),
				conn->sock.curservs->get_address(),
				conn->sock.curservs->get_port(), conn->createConnTime);

	record()->record_clientQueryAddNewClient(conn->createConnTime,
			conn->sock.curclins->get_addressHashCode(),
			conn->sock.curclins->get_address(), conn->sock.curclins->get_port(),
			conn->sock.curservs->get_address(), conn->sock.curservs->get_port());
	record()->record_acceptClientConn();
	record()->record_startHandingConn();

	unsigned int serverHashCode = Tool::quick_hash_code(conn->sock.curservs->get_address().c_str(),
			conn->sock.curservs->get_address().length());
	conn->sock.curservs->set_addressHashCode(serverHashCode);

	return conn;
}

void ParseData::free_connection(Connection* conn) {
	if (conn == NULL)
		return;

	if (conn->sock.curclins) {
		delete conn->sock.curclins;
		conn->sock.curclins = NULL;
	}

	if (conn->sock.curservs) {
		delete conn->sock.curservs;
		conn->sock.curservs = NULL;
	}

	if (conn->protocolBase) {
		conn->protocolBase->destoryInstance();
		conn->protocolBase = NULL;
	}

	delete conn;
	conn = NULL;
}

