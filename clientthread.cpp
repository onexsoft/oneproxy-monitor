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
#include "strategyfactory.h"
//#include "google/profiler.h"

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
	 ioEvent(IoEvent::get_instance(std::string("clientThread_ioEvent")))
{
	this->connManager = connManager;
	this->stop = false;
	this->startThread(ClientThread::start, this);
}

ClientThread::~ClientThread()
{
	this->joinThread();
	delete ioEvent;
	this->ioEvent = NULL;
}

void ClientThread::set_stop()
{
	this->stop = true;
}

bool ClientThread::get_stop()
{
	return this->stop;
}

void ClientThread::add_task2Queue(NetworkSocket* ns)
{
	clientLock.lock();
	this->taskQueue.push(ns);
	clientLock.unlock();
	taskStat.recv_connection();
	record()->record_threadRecvConn(this->get_threadId());
}

unsigned int ClientThread::get_ConnectionNum()
{
	return this->connectTypeMap.size();
}

int ClientThread::add_task(NetworkSocket* ns)
{
	record()->record_startHandingConn();
	config()->update_globalTime();

	Connection *conn = new Connection();
	conn->createConnTime = config()->get_globalMillisecondTime();
	conn->activeTime = config()->get_globalSecondTime();//use global time, decrease call system api
	conn->clins() = ns;
	conn->sessData.clientInfo = record()->record_getClientQueryInfo(ns->get_addressHashCode());
	this->add_connectFdRelation(ns->get_fd(), conn);

	logs(Logger::DEBUG, "add client fd(%d) to ioevent", ns->get_fd());
	if (this->ioEvent->add_ioEventRead(ns->get_fd(), ClientThread::rw_frontData, this)) {
		logs(Logger::ERR, "add ioEventRead error, fd: %d", ns->get_fd());
		this->remove_connectFdRelation(ns->get_fd());
		record()->record_closeClientConn();
		record()->record_threadFailConn(this->get_threadId());
		taskStat.close_connection();
		delete ns;
		delete conn;
		return -1;
	}

	//记录此线程开始处理任务
	record()->record_threadStartHandingConn(this->get_threadId());
	taskStat.doing_connection();
	return 0;
}

void ClientThread::handle_readFrontData(unsigned int fd)
{
	logs(Logger::DEBUG, "handle_readFrontData(%d)", fd);
	//1. get connection struct from map
	Connection* con = this->get_connection(fd);
	if (con == NULL) {
		logs(Logger::ERR, "fd(%d) do not in connectionTypeMap error", fd);
		return;
	}
	con->activeTime = config()->get_globalSecondTime();

	//2.read data from front socket
	NetworkSocket *cns = con->clins();
	int ret = cns->read_data();
	if (ret < 0) {
		this->finished_connection(con, CONN_FINISHED_ERR);
		return;
	} else if (ret == 2) {
		this->finished_connection(con, CONN_FINISHED_NORMAL);
		return;
	}

	StringBuf& cnsRecvBuf = cns->get_recvData();
	if (cnsRecvBuf.get_length() == 0) {
		logs(Logger::ERR, "recv data error");
		return;
	}
	logs(Logger::DEBUG, "fd(%d) send (%d) bytes data to backend, clientHashCode: %u",
			fd, cnsRecvBuf.get_length(), con->sock.curclins->get_addressHashCode());
	logs_buf((char*)"front =====> backend", (void*)cnsRecvBuf.get_buf(), cnsRecvBuf.get_length());

	//3. analyse current data
	int tret = 0;
	if ((tret = this->parse_frontDataPacket(con))) {
		if (tret == -1) {
			logs(Logger::ERR, "don't recognition the packet "
				"and current don't have database information, so close the connection");
			this->finished_connection(con, CONN_FINISHED_ERR);
		} else if (tret == 1) {
			this->finished_connection(con, CONN_FINISHED_NORMAL);
		}
		return;
	}

	//4. if current socket is not connection to server, alloc server
	if (cnsRecvBuf.get_remailLength() > 0 && this->alloc_server(con)) {
		logs(Logger::ERR, "connect to server error, fd: %d", fd);
		return;
	}

	//5. write data to server
	this->send_data(*con, false);
}

void ClientThread::finished_connection(Connection *con, ConnFinishType type)
{
	bool isFreeClientSock = false;
	NetworkSocket *cns = con->clins();
	NetworkSocket *msns = con->sock.masters;
	NetworkSocket *ssns = con->sock.slavers;
	NetworkSocket *csns = con->sock.curservs;

	if (cns && csns) {//explain the client already get server connection
		record()->record_clientQueryOnLineTime(
				cns->get_addressHashCode(),
				config()->get_globalMillisecondTime() - con->createConnTime,
				con->sessData.clientInfo);
	}
	logs(Logger::DEBUG, "type: %d", type);
	if (cns != NULL && csns != NULL) {
		logs(Logger::DEBUG, "close client(%d) =====> server(%d)", cns->get_fd(), csns->get_fd());
	} else if (cns != NULL) {
		logs(Logger::DEBUG, "close client(%d)", cns->get_fd());
	} else if (csns != NULL) {
		logs(Logger::DEBUG, "close servs(%d)", csns->get_fd());
	}

	if (cns) {
		logs(Logger::DEBUG, "close client socked(%d)", cns->get_fd());
		//记录当前为离线
		record()->record_clientQueryOffLine(cns->get_addressHashCode(), con->sessData.clientInfo);
		record()->record_closeClientConn();
		taskStat.done_connection();
		close_fds(cns);
		con->sock.curclins = NULL;
		isFreeClientSock = true;
	}

	bool servIsInEvent = false;
	if (msns && msns->get_status() == SOCKET_STATUS_WORKING_T) {
		logs(Logger::DEBUG, "close master socked(%d)", msns->get_fd());
		close_ioEvent(msns);
	} else if (msns) {
		servIsInEvent = true;
	}

	if (ssns && ssns->get_status() == SOCKET_STATUS_WORKING_T) {
		logs(Logger::DEBUG, "close slave socket(%d)", ssns->get_fd());
		close_ioEvent(ssns);
	} else if (ssns) {
		servIsInEvent = true;
	}

	if (con->protocolBase != NULL && !servIsInEvent) {
		con->protocolBase->protocol_releaseBackendConnect(*con, type);
		con->protocolBase->destoryInstance();
	}

	if (!servIsInEvent) {
		delete con;
	}

	if (isFreeClientSock) {
		record()->record_threadFinishedConn(this->get_threadId());
		if (this->connManager && this->connManager->get_mutexLock()){
			this->connManager->get_mutexLock()->lock();
			this->connManager->get_mutexLock()->signal_mutexCond();
			this->connManager->get_mutexLock()->unlock();
		}
	}
}

void ClientThread::get_serverFailed(Connection *con)
{
	logs(Logger::WARNING, "get server failed,fd:%d", con->sock.curclins->get_fd());
	con->clins()->get_socketRecord().connServerFailed();
	if (con->clins()->get_socketRecord().get_connServerFailed()
			>= config()->get_tryConnServerTimes()) {
		record()->record_clientQueryAllocServerFail(
				con->clins()->get_addressHashCode(), con->sessData.clientInfo);
		logs(Logger::WARNING, "get server failed, close fd:%d", con->sock.curclins->get_fd());
		finished_connection(con, CONN_FINISHED_ERR);
	} else {
		if (con->sock.masters != NULL) {
			close_fds(con->sock.masters);
		}
		if (con->sock.slavers != NULL) {
			close_fds(con->sock.slavers);
		}
		if (con->protocolBase) {
			con->protocolBase->destoryInstance();
			con->protocolBase = NULL;
		}
		add_FailConnQueue(con);
		taskStat.doing_failConnection();
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
		this->add_connectFdRelation(con->servns()->get_fd(), con);
		logs(Logger::INFO, "add server fd(%d) to epoll", con->servns()->get_fd());
		this->ioEvent->add_ioEventRead(con->servns()->get_fd(), ClientThread::rw_backendData, this);
	}

	if (con->sock.curservs == con->sock.masters && con->sock.masters != NULL) {
		logs(Logger::INFO, "current client(%d) use masters(%d)",
				con->sock.curclins->get_fd(), con->sock.curservs->get_fd());
	} else if (con->sock.slavers != NULL && con->sock.curservs == con->sock.slavers) {
		logs(Logger::INFO, "current client(%d) use slavers(%d)",
				con->sock.curclins->get_fd(), con->sock.curservs->get_fd());
	}

	return 0;
}

int ClientThread::parse_frontDataPacket(Connection* con)
{
	logs(Logger::DEBUG, "parse_frontDataPacket");
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
				logs(Logger::DEBUG,
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

				ProtocolHandleRetVal retVal = base->protocol_front(*con);
				if (retVal == HANDLE_RETURN_FAILED_CLOSE_CONN) {
					logs(Logger::DEBUG, "close the connection");
					return -1;
				} else if (retVal == HANDLE_RETURN_NORMAL_CLOSE_CONN) {
					logs(Logger::DEBUG, "normal close conn");
					return 1;
				}
				return 0;
			}
		}
		return -1;
	} else {
		ProtocolHandleRetVal retVal = con->protocolBase->protocol_front(*con);
		if (retVal == HANDLE_RETURN_FAILED_CLOSE_CONN) {
			logs(Logger::DEBUG, "close the connection");
			return -1;
		} else if (retVal == HANDLE_RETURN_NORMAL_CLOSE_CONN) {
			logs(Logger::DEBUG, "normal close connection");
			return 1;
		}
	}
	return 0;
}

int ClientThread::get_databaseFromGroup(Connection& con)
{
	return StrategyFactory::get_strategy()->get_databaseFromGroup(con);
}

void ClientThread::handle_readBackendData(unsigned int fd)
{
	logs(Logger::INFO, "handle_readBackendData(%d)", fd);
	//1. get connection from map
	Connection* con = NULL;
	if ((con = this->get_connection(fd)) == NULL) {
		logs(Logger::INFO, "no fd(%d) in connectTypeMap", fd);
		return;
	}
	con->activeTime = config()->get_globalSecondTime();
	logs(Logger::DEBUG, "fd: %d, activeTime: %llu", fd, con->activeTime);

	//2. read data from backend
	NetworkSocket *sns = con->sock.get_curServSock();
	if (sns == NULL)
		return;

	int ret = sns->read_data();
	if (ret < 0) {
		this->finished_connection(con, CONN_FINISHED_ERR);
		return;
	} else if (ret == 2) {
		this->finished_connection(con, CONN_FINISHED_NORMAL);
		return;
	}
	if (sns->get_recvData().get_length() == 0) {
		return;
	}

	if (con->sock.curclins != NULL) {//when in the case that clean backend socket
		logs(Logger::DEBUG, "fd(%d) send (%d) bytes data to front, clientHashCode: %u",
				fd,  sns->get_recvData().get_length(), con->sock.curclins->get_addressHashCode());
	} else {
		logs(Logger::DEBUG, "fd(%d) send(%d)bytes data to oneproxy", fd, sns->get_recvData().get_length());
	}
	logs_buf((char*)"backend =====> front",(void*)sns->get_recvData().get_buf(), sns->get_recvData().get_length());

	//3. parse backend packet
	int tret = 0;
	if ((tret = this->parse_backendDataPacket(con))) {
		if (tret == 1) {
			this->finished_connection(con, CONN_FINISHED_NORMAL);
		} else {
			this->finished_connection(con, CONN_FINISHED_ERR);
		}
		return ;
	}

	//4. send data
	this->send_data(*con, true);
}

int ClientThread::parse_backendDataPacket(Connection* con) {
	assert(con != NULL);
	assert(con->protocolBase != NULL);
	ProtocolHandleRetVal retVal = con->protocolBase->protocol_backend(*con);
	if (retVal == HANDLE_RETURN_FAILED_CLOSE_CONN) {
		return -1;
	} else if (retVal == HANDLE_RETURN_NORMAL_CLOSE_CONN) {
		return 1;
	}
	return 0;
}

int ClientThread::send_data(Connection& conn, bool sendToClient) {

	if (sendToClient) {
		NetworkSocket* sns = conn.sock.curservs;
		if (sns != NULL) {
			if (sns->get_recvData().get_remailLength() > 0) {
				this->write_data(conn, true);
			}
			if (sns->get_sendData().get_remailLength() > 0) {
				this->write_data(conn, false);
			}
		}

		NetworkSocket* cns = conn.sock.curclins;
		if (cns != NULL) {
			if (cns->get_sendData().get_remailLength() > 0) {
				this->write_data(conn, true);
			}
		}
	} else {
		NetworkSocket* cns = conn.sock.curclins;
		if (cns != NULL) {
			if (cns->get_recvData().get_remailLength() > 0) {
				this->write_data(conn, false);
			}

			if (cns->get_sendData().get_remailLength() > 0) {
				this->write_data(conn, true);
			}
		}

		//add by huih@20161220
		NetworkSocket* sns = conn.sock.curservs;
		if (sns != NULL) {
			if (sns->get_sendData().get_remailLength() > 0) {
				this->write_data(conn, false);
			}
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
	NetworkSocket* sSocket = NULL; //source socket.
	Func sFunc = NULL;

	if (isFront) {
		tSocket = con.sock.get_clientSock();
		sSocket = con.sock.get_curServSock();
		if (con.sock.get_curServSock() != NULL) {
			tData = &(con.sock.get_curServSock()->get_recvData());
		} else {
			tData = &emptyPacket;
		}
		func = ClientThread::rw_frontData;
		sFunc = ClientThread::rw_backendData;
	} else {
		tSocket = con.sock.get_curServSock();
		sSocket = con.sock.get_clientSock();
		if (con.sock.get_clientSock() != NULL) {
			tData = &(con.sock.get_clientSock()->get_recvData());
		} else {
			tData = &emptyPacket;
		}
		func = ClientThread::rw_backendData;
		sFunc = ClientThread::rw_frontData;
	}
	NetworkSocket& desSocket = *tSocket;
	StringBuf& data = *tData;

	//first write sendData
	StringBuf& sendData = desSocket.get_sendData();
	uif (sendData.get_remailLength() <= 0 && data.get_remailLength() <= 0) {
		data.clear();
		sendData.clear();
		return;
	}

#define save_data_toSendData() do{\
	logs(Logger::INFO, "rewrite data ...., fd:%d, data.offset: %d, "\
		"data.length: %d, sendBuf.length: %d", desSocket.get_fd(),\
			data.get_offset(), data.length(), desSocket.get_sendData().get_length());\
	desSocket.save_data(data);\
	this->ioEvent->add_ioEventWrite(desSocket.get_fd(), func, this);\
	if (sSocket != NULL) this->ioEvent->del_ioEvent(sSocket->get_fd());\
}while(0)

	do {
		//先写上次没有发送完毕的数据
		uif (sendData.get_remailLength() > 0) {
			logs(Logger::INFO, "send data : %d bytes", sendData.get_remailLength());
			ret = desSocket.write_data(sendData);
			if (ret == 1) {//无法写入数据
				save_data_toSendData();
				break;
			} else if (ret) {//error
				this->finished_connection(&con, CONN_FINISHED_ERR);
				return;
			} else {//write finished.
				sendData.clear();
				if (sSocket != NULL)
					this->ioEvent->add_ioEventRead(sSocket->get_fd(), sFunc, this);
			}
		}

		ret = desSocket.write_data(data);
		if (ret == 1) {
			save_data_toSendData();
			break;
		} else uif (ret) {
			logs(Logger::ERR, "write data to (%d) error", desSocket.get_fd());
			this->finished_connection(&con, CONN_FINISHED_ERR);
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
	ClientThread::ConnectionTypeMap::iterator it = this->connectTypeMap.find(fd);
	if (it != this->connectTypeMap.end()) {
		conn = it->second;
	} else {
		logs(Logger::INFO, "no fd(%d) in connectTypeMap(%d)", fd, this->connectTypeMap.size());
	}
	return conn;
}

void ClientThread::add_connectFdRelation(unsigned int fd, Connection* con) {
	this->connectTypeMap[fd] = con;
}

void ClientThread::remove_connectFdRelation(unsigned int fd)
{
	this->connectTypeMap.erase(fd);
	logs(Logger::DEBUG, "remove fd(%d) to connectTypeMap(size:%d)", fd, this->connectTypeMap.size());
}

void ClientThread::handle_taskQueue()
{
	if (this->taskQueue.size() <= 0)
		return;

	NetworkSocket* ns = NULL;
	this->clientLock.lock();
	while(this->taskQueue.size() > 0) {//batch add task to epoll.
		ns = this->taskQueue.front();
		this->taskQueue.pop();
		if (ns != NULL)
			this->add_task(ns);
	}
	this->clientLock.unlock();
}

void ClientThread::add_FailConnQueue(Connection* conn)
{
	lif(conn && conn->sock.curclins)
		logs(Logger::WARNING, "client socket: %d", conn->sock.curclins->get_fd());
	this->FailConnQueue.push(conn);
}

void ClientThread::handle_FailConnQueue()
{
	if (FailConnQueue.size() > 0) {
		Connection* conn = FailConnQueue.front();
		FailConnQueue.pop();
		if (conn && conn->sock.curclins) {
			this->handle_readFrontData(conn->sock.curclins->get_fd());
			taskStat.retry_failConnection();
		}
	}
}

bool ClientThread::have_queueData()
{
	if (this->taskQueue.size() || this->FailConnQueue.size())
		return true;
	return false;
}

void ClientThread::handle_queueData()
{
	if (this->FailConnQueue.size())
		this->handle_FailConnQueue();

	if (this->taskQueue.size())
		this->handle_taskQueue();
}

void ClientThread::check_connectionTimeout() {
	Connection* conn = NULL;
	ConnectionTypeMap::iterator it = this->connectTypeMap.begin();
	for (; it != this->connectTypeMap.end(); ++it) {
		if ((config()->get_globalSecondTime() - it->second->activeTime) > (u_uint64)(config()->get_connectTimeOut())) {//one day
			conn = it->second;
			break;//one time, close one connection.
		}
	}
	if (conn != NULL) {
		logs(Logger::WARNING, "timeout close client fd: %d", conn->sock.curclins->get_fd());
		this->finished_connection(conn, CONN_FINISHED_ERR);
	}
}

unsigned int ClientThread::get_threadTaskNum()
{
	return taskStat.sum_connection();
}

void ClientThread::rw_frontData(unsigned int fd, unsigned int events, void* args)
{
	ClientThread* ct = (ClientThread*)args;
	logs(Logger::INFO, "front fd(%d) have data to read", fd);
	config()->update_globalTime();
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
	config()->update_globalTime();
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
//	ProfilerRegisterThread();
	ClientThread *ct = (ClientThread*)args;
	unsigned int cntTime = 10;//5 second, check connection one times.
	while(ct->get_stop() == false || ct->get_ConnectionNum() || ct->have_queueData()) {
		if (ct->have_queueData()) {
			ct->ioEvent->run_loopWithTimeout(0);
			ct->handle_queueData();
		} else {
			ct->ioEvent->run_loopWithTimeout(100);
		}

		if (cntTime > 0) {
			cntTime = cntTime - 1;
		} else {
			cntTime = 10;
			ct->check_connectionTimeout();
		}
	}
	return 0;
}
