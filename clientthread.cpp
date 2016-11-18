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

#define close_fds(sock) do{\
	this->ioEvent->del_ioEvent(sock->get_fd());\
	this->remove_connectFdRelation(sock->get_fd());\
	delete sock;\
	sock = NULL;\
}while(0)

#define close_ioEvent(sock) do{\
	this->ioEvent->del_ioEvent(sock->get_fd());\
	this->remove_connectFdRelation(sock->get_fd());\
}while(0)



ClientThread::ClientThread(ConnectManager* connManager, std::string threadName)
	:Thread(thread_type_client, threadName),
	 ioEvent(IoEvent::get_instance(std::string("clientThread_ioEvent"))),
	 clientLock(std::string("clientThread_lock"), NULL)
{
	this->connManager = connManager;
	this->stop = false;
	this->startThread(ClientThread::start, this);
}

ClientThread::~ClientThread()
{
	delete ioEvent;
}

void ClientThread::set_stop()
{
	this->stop = true;
}

bool ClientThread::get_stop()
{
	return this->stop;
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
		this->remove_connectFdRelation(ns->get_fd());
		delete conn;
		return -1;
	}

	//记录此线程开始处理任务
	record()->record_threadStartHandingConn(this->get_threadId());
	return 0;
}

unsigned int ClientThread::get_ConnectionNum()
{
	return this->connectTypeMap.size();
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
	if (cns->read_data() < 0) {
		this->finished_connection(con);
		return;
	}
	StringBuf& cnsRecvBuf = cns->get_recvData();
	if (cnsRecvBuf.get_length() == 0) {
		return;
	}
	logs(Logger::INFO, "send (%d) bytes data to backend", cnsRecvBuf.get_length());
	logs_buf((char*)"front =====> backend", (void*)cnsRecvBuf.get_buf(), cnsRecvBuf.get_length());

	//3. analyse current data
	if (this->parse_frontDataPacket(con)) {
		logs(Logger::DEBUG, "don't recognition the packet "
				"and current don't have database information, so close the connection");
		this->finished_connection(con);
		return;
	}

	//4. if current socket is not connection to server, alloc server
	if (cnsRecvBuf.get_remailLength() > 0 && this->alloc_server(con)) {
		logs(Logger::ERR, "connect to server error");
		return;
	}

	//5. write data to server
	this->send_data(*con, false);
}

void ClientThread::finished_connection(Connection *con)
{
	NetworkSocket *cns = con->clins();
	NetworkSocket *msns = con->sock.masters;
	NetworkSocket *ssns = con->sock.slavers;
	NetworkSocket *csns = con->sock.curservs;

	if (cns == NULL || cns->get_fd() <= 0)
		return;

	if (csns) {//explain the client already get server connection
		u_uint64 endTime = SystemApi::system_millisecond();
		record()->clientQueryMap[cns->get_addressHashCode()].part.onLineTime
				+= (endTime - con->createConnTime);
	}
	if (cns != NULL && csns != NULL) {
		logs(Logger::DEBUG, "close client(%d) =====> server(%d)", cns->get_fd(), csns->get_fd());
	} else if (cns != NULL) {
		logs(Logger::DEBUG, "close client(%d)", cns->get_fd());
	} else {
		logs(Logger::DEBUG, "close client(%d)", csns->get_fd());
	}

	if (cns) {
		logs(Logger::DEBUG, "close client socked(%d)", cns->get_fd());
		//记录当前为离线
		record()->clientQueryMap[cns->get_addressHashCode()].part.onLineStatus = false;
		record()->clientQueryMap[cns->get_addressHashCode()].part.start_connect_time = 0;//断开连接时，设置为0.再排序时，排在最后
		this->connManager->finished_task(this->get_threadId(), cns);
		close_fds(cns);
	}

	if (msns) {
		logs(Logger::DEBUG, "close master socked(%d)", msns->get_fd());
		close_ioEvent(msns);
	} else {
		logs(Logger::DEBUG, "msns == NULL");
	}
	if (ssns) {
		logs(Logger::DEBUG, "close slave socket(%d)", ssns->get_fd());
		close_ioEvent(ssns);
	} else {
		logs(Logger::DEBUG, "ssns == NULL");
	}

	if (con->protocolBase != NULL) {
		con->protocolBase->protocol_releaseBackendConnect(*con);
		con->protocolBase->destoryInstance();
	}

	delete con;
	record()->record_threadFinishedConn(this->get_threadId());
}

void ClientThread::get_serverFailed(Connection *con)
{
	if (con->sock.masters != NULL) {
		logs(Logger::DEBUG, "delete master socket: %d", con->sock.masters->get_fd());
		close_fds(con->sock.masters);
	}
	if (con->sock.slavers != NULL){
		logs(Logger::DEBUG, "delete slavers socket: %d", con->sock.slavers->get_fd());
		close_fds(con->sock.slavers);
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
						"slaveGroup: %s, frontPort: %d, passwordSeparate: %d, readSlave: %d, "
						"useConnectionPool: %d", dbg->get_labelName().c_str(),
						dbg->get_className().c_str(), dbg->get_dbMasterGroup().c_str(),
						dbg->get_dbSlaveGroup().c_str(), dbg->get_frontPort(),
						dbg->get_passwordSeparate(), dbg->get_readSlave(), dbg->get_useConnectionPool());
				con->database.dataBaseGroup = dbg;
				uif(base->protocol_init(*con)) {//环境初始化失败
					con->database.dataBaseGroup = NULL;
					return -1;
				}
				con->protocolBase = base;

				//先选择数据库，在进行解析数据包，防止解析失败的情况
				if (this->get_databaseFromGroup(*con)) {
					return -1;
				}

				if (base->protocol_front(*con) == HANDLE_RETURN_FAILED_CLOSE_CONN) {
					logs(Logger::DEBUG, "close the connection");
					return -1;
				}
				return 0;
			}
		}
		return -1;
	} else {
		if (con->protocolBase->protocol_front(*con) == HANDLE_RETURN_FAILED_CLOSE_CONN) {
			logs(Logger::DEBUG, "close the connection");
			return -1;
		}
	}
	return 0;
}

int ClientThread::get_databaseFromGroup(Connection& con)
{
	if (con.database.dataBaseGroup->get_dbMasterGroupVec().size() > 0) {
		con.database.masterDataBase = con.database.dataBaseGroup->get_dbMasterGroupVec().front();
	} else {//必须需要master
		logs(Logger::ERR, "no master database in config file");
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
		finished_connection(con);
		return ;
	}

	//4. send data
	this->send_data(*con, true);
}

int ClientThread::parse_backendDataPacket(Connection* con) {
	assert(con != NULL);
	assert(con->protocolBase != NULL);
	if (con->protocolBase->protocol_backend(*con) == HANDLE_RETURN_FAILED_CLOSE_CONN) {
		return -1;
	}
	return 0;
}

int ClientThread::send_data(Connection& conn, bool sendToClient) {

	if (sendToClient) {
		NetworkSocket* ns = conn.sock.curservs;
		if (ns == NULL)
			return 0;

		if (ns->get_recvData().get_remailLength() > 0) {
			this->write_data(conn, true);
		}

		if (ns->get_sendData().get_remailLength() > 0) {
			this->write_data(conn, false);
		}
	} else {
		NetworkSocket* ns = conn.sock.curclins;
		if (ns == NULL)
			return 0;

		if (ns->get_recvData().get_remailLength() > 0) {
			this->write_data(conn, false);
		}

		if (ns->get_sendData().get_remailLength() > 0) {
			this->write_data(conn, true);
		}
	}

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
			logs(Logger::INFO, "send data : %d bytes", sendData.get_remailLength());
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
			this->ioEvent->add_ioEventRead(desSocket.get_fd(), func, this);
			break;
		}
	} while(0);

	data.clear();
}

Connection* ClientThread::get_connection(unsigned int fd)
{
	Connection* conn = NULL;
	this->clientLock.lock();
	ClientThread::ConnectionTypeMap::iterator it = this->connectTypeMap.find(fd);
	if (it != this->connectTypeMap.end()) {
		conn = it->second;
	} else {
		logs(Logger::INFO, "no fd(%d) in connectTypeMap(%d)", fd, this->connectTypeMap.size());
	}
	this->clientLock.unlock();
	return conn;
}

void ClientThread::remove_connectFdRelation(unsigned int fd)
{
	this->clientLock.lock();
	this->connectTypeMap.erase(fd);
	logs(Logger::INFO, "remove fd(%d) to connectTypeMap(size:%d)", fd, this->connectTypeMap.size());
	this->clientLock.unlock();
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
		logs(Logger::DEBUG, "is_readEvent");
		ct->handle_readBackendData(fd);
	} else if (ct->ioEvent->is_writeEvent(events)) {
		logs(Logger::DEBUG, "is_writeEvent");
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
	ClientThread *ct = (ClientThread*)args;

	while(ct->get_stop() == false || ct->get_ConnectionNum()) {
		ct->ioEvent->run_loopWithTimeout(500);//500 ms
	}
	return 0;
}
