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
* @FileName: clientthread.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月4日
*
*/

#include "clientthread.h"


#include "protocoldynamiccls.h"
#include "connectmanager.h"
#include "protocolfactory.h"
#include "protocolbase.h"
#include "record.h"
#include "config.h"
#include "tool.h"

ClientThread::ClientThread(ConnectManager* connManager, std::string threadName)
	:Thread(thread_type_client, threadName),
	 ioEvent(IoEvent::get_instance(std::string("clientThread_ioEvent"))),
	 clientLock(std::string("clientThread_lock"), record())
{
	this->connManager = connManager;
	this->startThread(ClientThread::start, this);
}

ClientThread::~ClientThread()
{
	delete ioEvent;
}

void ClientThread::set_stop()
{
	this->ioEvent->stop_event();
}

int ClientThread::add_task(NetworkSocket* ns)
{
	Connection *conn = new Connection();
	conn->createConnTime = SystemApi::system_millisecond();
	conn->clins() = ns;

	clientLock.lock();
	this->connectTypeMap[ns->get_fd()] = conn;
	clientLock.unlock();

	logs(Logger::INFO, "add client fd(%d) to ioevent", ns->get_fd());
	if (this->ioEvent->add_ioEventRead(ns->get_fd(), ClientThread::rw_frontData, this)) {
		clientLock.lock();
		this->connectTypeMap.erase(ns->get_fd());
		clientLock.unlock();
		delete conn;
		return -1;
	}

	//记录此线程开始处理任务
	record()->record_threadStartHandingConn(this->get_threadId());
	return 0;
}

void ClientThread::handle_readFrontData(unsigned int fd)
{
	logs(Logger::INFO, "handle_readFrontData");
	//1. get connection struct from map
	Connection* con = this->get_connection(fd);
	if (con == NULL) {
		logs(Logger::ERR, "fd(%d) do not in connectionTypeMap error", fd);
		return;
	}

	//2.read data from front socket
	NetworkSocket *cns = con->clins();
	if (cns == NULL) {
		logs(Logger::ERR, "cns is null");
		return;
	}
	if (cns->read_data() < 0) {
		this->finished_connection(con);
		return;
	}
	if (cns->get_recvData().get_length() == 0) {
		return;
	}

	//3. analyse current data
	if (this->parse_frontDataPacket(con)) {
		logs(Logger::ERR, "parse packet error");
		if (con->database.dataBaseGroup == NULL || con->protocolBase == NULL) {
			logs(Logger::ERR, "don't recognition the packet and current don't have database information, so close the connection");
			this->finished_connection(con);
			return;
		}
	}

	//4.check: Have data need send to server or not
	if (cns->get_recvData().get_remailLength() <= 0) {
		if (cns->get_sendData().get_remailLength() > 0) {//maybe have data need send to client
			this->write_data(*con, true);
		}
		return;
	}

	//4. if current socket is not connection to server, alloc server
	if (this->alloc_server(con)) {
		logs(Logger::ERR, "connect to server error");
		return;
	}

	logs(Logger::INFO, "send (%d) bytes data to backend", cns->get_recvData().get_length());
	logs_buf((char*)"front =====> backend", (void*)cns->get_recvData().get_buf(), cns->get_recvData().get_length());

	//5. write data to server
	this->write_data(*con, false);
}

void ClientThread::finished_connection(Connection *con)
{
	NetworkSocket *cns = con->clins();
	NetworkSocket *msns = con->sock.masters;
	NetworkSocket *ssns = con->sock.slavers;
	NetworkSocket *csns = con->sock.curservs;

	if (csns) {//explain the client already get server connection
		u_uint64 endTime = SystemApi::system_millisecond();
		record()->clientQueryMap[cns->get_addressHashCode()].part.onLineTime
				+= (endTime - con->createConnTime);
	}

	if (cns) {
		//记录当前为离线
		record()->clientQueryMap[cns->get_addressHashCode()].part.onLineStatus = false;
		record()->clientQueryMap[cns->get_addressHashCode()].part.start_connect_time = 0;//断开连接时，设置为0.再排序时，排在最后
		this->ioEvent->del_ioEvent(cns->get_fd());
		this->connManager->finished_task(this->get_threadId(), cns);
		clientLock.lock();
		this->connectTypeMap.erase(cns->get_fd());
		logs(Logger::INFO, "remove fd(%d) to connectTypeMap(%d)", cns->get_fd(), this->connectTypeMap.size());
		clientLock.unlock();
		delete cns;
	}

	if (msns) {
		this->ioEvent->del_ioEvent(msns->get_fd());
		clientLock.lock();
		this->connectTypeMap.erase(msns->get_fd());
		logs(Logger::INFO, "remove fd(%d) to connectTypeMap(%d)", msns->get_fd(), this->connectTypeMap.size());
		clientLock.unlock();
		delete msns;
	}

	if (ssns) {
		this->ioEvent->del_ioEvent(ssns->get_fd());
		clientLock.lock();
		this->connectTypeMap.erase(ssns->get_fd());
		logs(Logger::INFO, "remove fd(%d) to connectTypeMap(%d)", ssns->get_fd(), this->connectTypeMap.size());
		clientLock.unlock();
		delete ssns;
	}

	if (con->protocolBase != NULL) {
		con->protocolBase->destoryInstance();
	}
	delete con;

	record()->record_threadFinishedConn(this->get_threadId());
}

void ClientThread::get_serverFailed(Connection *con)
{
	if (con->sock.masters != NULL){
		delete con->sock.masters;
		con->sock.masters = NULL;
	}
	if (con->sock.slavers != NULL){
		delete con->sock.slavers;
		con->sock.slavers = NULL;
	}

	con->clins()->get_socketRecord().connServerFailed();
	if (con->clins()->get_socketRecord().get_connServerFailed()
			>= config()->get_tryConnServerTimes()) {
		record()->clientQueryMap[con->clins()->get_addressHashCode()].part.connServerFail++;
		finished_connection(con);
	} else {
		this->ioEvent->del_ioEvent(con->clins()->get_fd());
		this->connManager->finished_task(this->get_threadId(), con->clins());
		this->connManager->add_task(con->clins());
		if (con->protocolBase) {
			con->protocolBase->destoryInstance();
		}
		delete con;
		record()->record_threadFailConn(this->get_threadId());
	}
}

int ClientThread::alloc_server(Connection* con)
{
	//connection to server
	if (con->servns() == NULL) {
		if (con->protocolBase->protocol_getBackendConnect(*con)) {
			get_serverFailed(con);
			return -1;
		}
		clientLock.lock();
		this->connectTypeMap[con->servns()->get_fd()] = con;
		clientLock.unlock();
		logs(Logger::INFO, "add server fd(%d) to epoll", con->servns()->get_fd());
		this->ioEvent->add_ioEventRead(con->servns()->get_fd(), ClientThread::rw_backendData, this);
	}
	return 0;
}

int ClientThread::parse_frontDataPacket(Connection* con)
{
	logs(Logger::INFO, "parse_frontDataPacket");
	NetworkSocket* ns = con->clins();
	if (con != NULL && con->protocolBase == NULL) {
		unsigned int i = 0;
		for (i = 0; i < config()->get_dataBaseGroupSize(); ++i) {
			DataBaseGroup* dbg = config()->get_dataBaseGroup(i);
			if (dbg->get_frontPort() != 0 && dbg->get_frontPort() != ns->get_attachData().get_listenPort())
				continue;

			ProtocolBase* base =
					(ProtocolBase*)ProtocolFactory::sharedClassFactory().getClassByName(dbg->get_className());
			if(base != NULL && base->is_currentDatabase(*con)) {
				logs(Logger::INFO,
						"current database is: labelName %s, className %s, masterGroup: %s, "
						"slaveGroup: %s, frontPort: %d", dbg->get_labelName().c_str(),
						dbg->get_className().c_str(), dbg->get_dbMasterGroup().c_str(),
						dbg->get_dbSlaveGroup().c_str(), dbg->get_frontPort());
				uif(base->protocol_init(*con)) {//环境初始化失败
					return -1;
				}
				con->database.dataBaseGroup = dbg;
				con->protocolBase = base;
				base->protocol_front(*con);
				return this->get_databaseFromGroup(*con);
			}
		}
		return -1;
	} else {
		con->protocolBase->protocol_front(*con);
	}
	return 0;
}

int ClientThread::get_databaseFromGroup(Connection& con)
{
	if (con.database.dataBaseGroup->get_dbMasterGroupVec().size() > 0) {
		con.database.masterDataBase = con.database.dataBaseGroup->get_dbMasterGroupVec().front();
	} else {//必须需要master
		return -1;
	}

	if (con.database.dataBaseGroup->get_dbSlaveGroupVec().size() > 0) {//可以没有slave
		con.database.slaveDataBase = con.database.dataBaseGroup->get_dbSlaveGroupVec().front();
	}

	//first set current database is master database
	con.database.currentDataBase = con.database.masterDataBase;
	return 0;
}

void ClientThread::handle_readBackendData(unsigned int fd)
{
	logs(Logger::INFO, "handle_readBackendData");
	//1. get connection from map
	Connection* con = NULL;
	if ((con = this->get_connection(fd)) == NULL) {
		logs(Logger::INFO, "no fd(%d) in connectTypeMap", fd);
		return;
	}

	//2. read data from backend
	NetworkSocket *sns = con->sock.get_curServSock();
	if (sns == NULL)
		return;

	if (sns->read_data() < 0) {
		this->finished_connection(con);
		return;
	}
	if (sns->get_recvData().get_length() == 0)
		return;

	logs(Logger::INFO, "send (%d) bytes data to front", sns->get_recvData().get_length());
	logs_buf((char*)"backend =====> front",(void*)sns->get_recvData().get_buf(), sns->get_recvData().get_length());

	//3. parse backend packet
	if (this->parse_backendDataPacket(con)) {
		logs(Logger::ERR, "parse packet error");
	}

	//4. write data to front
	this->write_data(*con, true);
}

int ClientThread::parse_backendDataPacket(Connection* con) {
	assert(con != NULL);
	assert(con->protocolBase != NULL);
	con->protocolBase->protocol_backend(*con);
	return 0;
}

void ClientThread::write_data(Connection& con, bool isFront)
{
	int ret = 0;
	NetworkSocket* tSocket = NULL;
	StringBuf* tData = NULL;
	StringBuf emptyPacket;
	Func func = NULL;

	if (isFront) {
		tSocket = con.sock.get_clientSock();
		if (con.sock.get_curServSock() != NULL) {
			tData = &(con.sock.get_curServSock()->get_recvData());
		} else {
			tData = &emptyPacket;
		}
		func = ClientThread::rw_frontData;
	} else {
		tSocket = con.sock.get_curServSock();
		tData = &(con.sock.get_clientSock()->get_recvData());
		func = ClientThread::rw_backendData;
	}
	NetworkSocket& desSocket = *tSocket;
	StringBuf& data = *tData;

	//first write sendData
	StringBuf& sendData = desSocket.get_sendData();
	uif (sendData.get_remailLength() <= 0 && data.get_remailLength() <= 0)
		return;

	do {
		//先写上次没有发送完毕的数据
		uif (sendData.get_remailLength() > 0) {
			logs(Logger::INFO, "send data : %d", sendData.get_remailLength());
			ret = desSocket.write_data(sendData);
			if (ret == 1) {//无法写入数据
				desSocket.save_data(data);
				this->ioEvent->add_ioEventWrite(desSocket.get_fd(), func, this);
				break;
			} else if (ret) {//error
				this->finished_connection(&con);
				return;
			} else {//write finished.
				sendData.clear();
			}
		}
		uif (sendData.length() > 0) {
			sendData.clear();
		}

		ret = desSocket.write_data(data);
		if (ret == 1) {
			logs(Logger::INFO, "rewrite data ...., fd:%d, data.offset: %d, data.length: %d", desSocket.get_fd(),
					data.get_offset(), data.length());
			//save data to try
			desSocket.save_data(data);
			this->ioEvent->add_ioEventWrite(desSocket.get_fd(), func, this);
			break;
		} else uif (ret) {
			logs(Logger::ERR, "write data to (%d) error", desSocket.get_fd());
			this->finished_connection(&con);
			return;
		} else {//写数据完毕
			logs(Logger::INFO, "write finished ...");
			break;
		}
	} while(0);

	data.clear();
}

Connection* ClientThread::get_connection(unsigned int fd)
{
	ClientThread::ConnectionTypeMap::iterator it = this->connectTypeMap.find(fd);
	if (it == this->connectTypeMap.end()) {
		logs(Logger::INFO, "no fd(%d) in connectTypeMap(%d)", fd, this->connectTypeMap.size());
		return NULL;
	}
	return it->second;
}

void ClientThread::rw_frontData(unsigned int fd, unsigned int events, void* args)
{
	ClientThread* ct = (ClientThread*)args;
	logs(Logger::INFO, "front fd(%d) have data to read", fd);
	if (ct->ioEvent->is_readEvent(events)) {
		ct->handle_readFrontData(fd);
	} else if (ct->ioEvent->is_writeEvent(events)) {
		Connection* con = NULL;
		if ((con = ct->get_connection(fd)) == NULL) {
			logs(Logger::ERR, "no fd(%d) in connectTypeMap", fd);
			return;
		}
		ct->write_data(*con, true);
	} else {
		logs(Logger::ERR, "unknow events(%d)", events);
	}
}

void ClientThread::rw_backendData(unsigned int fd, unsigned int events, void *args)
{
	ClientThread* ct = (ClientThread*)args;

	logs(Logger::INFO, "backend fd(%d) have data to read, events(%d)", fd, events);
	if (ct->ioEvent->is_readEvent(events)) {
		ct->handle_readBackendData(fd);
	} else if (ct->ioEvent->is_writeEvent(events)) {
		Connection* con = NULL;
		if ((con = ct->get_connection(fd)) == NULL) {
			logs(Logger::ERR, "no fd(%d) in connectTypeMap", fd);
			return;
		}
		ct->write_data(*con, false);
	} else {
		logs(Logger::ERR, "unknow events(%d)", events);
	}
}

thread_start_func(ClientThread::start)
{
	logs(Logger::INFO, "start client Thread");
	ClientThread *ct = (ClientThread*)args;

	ct->ioEvent->run_loop(500);//500ms

	return 0;
}
