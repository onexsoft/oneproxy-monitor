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
#include "sys/types.h"
#ifndef __WIN32
#include "sys/socket.h"
#endif
#include "socketutil.h"
//#include "google/profiler.h"

#define close_fds(sock) do{\
	this->ioEvent->del_ioEvent(sock->get_fd());\
	delete sock;\
	sock = NULL;\
}while(0)

ClientThread::ClientThread(ConnectManager* connManager, std::string threadName)
	:Thread(thread_type_client, threadName),
	 ioEvent(IoEvent::get_instance(std::string("clientThread_ioEvent")))
{
	this->connManager = connManager;
	this->stop = false;

	uif(SystemApi::system_socketpair(AF_LOCAL, SOCK_STREAM, 0, this->spfd)) {
		logs(Logger::FATAL, "create socketpair error(%s)", SystemApi::system_strerror());
	}
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
	this->ioEvent->stop_event();
}

bool ClientThread::get_stop()
{
	return this->stop;
}

unsigned int ClientThread::get_ConnectionNum()
{
	return this->connectList.size();
}

int ClientThread::add_task(NetworkSocket* ns)
{
	record()->record_startHandingConn();
	config()->update_globalTime();

	Connection *conn = new Connection();
	conn->createConnTime = ns->get_attachData().get_connTime();
	conn->connection_hashcode = ns->get_attachData().get_connHashCode();
	conn->activeTime = config()->get_globalSecondTime();//use global time, decrease call system api
	conn->clins() = ns;
	conn->sessData.clientInfo = record()->record_getClientQueryInfo(ns->get_addressHashCode());
	conn->clientThreadObj = (void*)this;

	logs(Logger::DEBUG, "add client fd(%d) to ioevent", ns->get_fd());
	if (this->ioEvent->add_ioEventRead(ns->get_fd(), ClientThread::rw_frontData, conn)) {
		logs(Logger::ERR, "add ioEventRead error, fd: %d", ns->get_fd());
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
	connectList.push_back(conn);
	return 0;
}

void ClientThread::handle_readFrontData(Connection& conn)
{
	uif (conn.sock.curclins == NULL)
		return;
	logs(Logger::DEBUG, "handle_readFrontData(%d)", conn.sock.curclins->get_fd());

	conn.activeTime = config()->get_globalSecondTime();

	//2.read data from front socket
	NetworkSocket *cns = conn.clins();
	int ret = cns->read_data();
	if (ret < 0) {
		logs(Logger::WARNING, "read data error, close connection");
		this->finished_connection(&conn, CONN_FINISHED_ERR);
		return;
	} else if (ret == 2 && cns->get_recvData().get_remailLength() <= 0) {
		/**
		 * 客户端强制关闭，服务端的状态可能不正确，是否复用由底层协议决定
		 * 如果是获取后端失败，重试的情况下，前端socket没有数据，但是socket已经在上次读取到数据了
		 * 需要从新进行处理。
		 * **/
		this->finished_connection(&conn, CONN_FINISHED_FRONT_CLOSE);
		return;
	}

	StringBuf& cnsRecvBuf = cns->get_recvData();
	if (cnsRecvBuf.get_length() == 0) {
		logs(Logger::ERR, "recv data error");
		return;
	}
	logs(Logger::DEBUG, "fd(%d) send (%d) bytes data to backend, clientHashCode: %u",
			conn.sock.curclins->get_fd(), cnsRecvBuf.get_length(),
			conn.sock.curclins->get_addressHashCode());
	logs_buf((char*)"front =====> backend", (void*)cnsRecvBuf.get_buf(), cnsRecvBuf.get_length());

	//3. analyse current data
	int tret = 0;
	if ((tret = this->parse_frontDataPacket(&conn))) {
		if (tret == -1) {
			logs(Logger::WARNING, "parse front data packet error, close the connection.");
			this->finished_connection(&conn, CONN_FINISHED_ERR);
		} else if (tret == 1) {
			this->finished_connection(&conn, CONN_FINISHED_NORMAL);
		}
		return;
	}

	//4. if current socket is not connection to server, alloc server
	if (cnsRecvBuf.get_remailLength() > 0 && this->alloc_server(&conn)) {
		logs(Logger::ERR, "connect to server error, fd: %d", conn.sock.curclins->get_fd());
		return;
	}

	//5. write data to server
	this->send_data(conn, false);
}

void ClientThread::finished_connection(Connection *con, ConnFinishType type)
{
	bool isFreeClientSock = false;
	NetworkSocket *cns = con->clins();
	NetworkSocket *msns = con->sock.masters;
	NetworkSocket *ssns = con->sock.slavers;
	NetworkSocket *csns = con->sock.curservs;

	if (cns && (msns || ssns) && csns) {//explain the client already get server connection
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
		this->ioEvent->del_ioEvent(msns->get_fd());
	} else if (msns) {
		servIsInEvent = true;
	}

	if (ssns && ssns->get_status() == SOCKET_STATUS_WORKING_T) {
		logs(Logger::DEBUG, "close slave socket(%d)", ssns->get_fd());
//		close_ioEvent(ssns);
		this->ioEvent->del_ioEvent(ssns->get_fd());
	} else if (ssns) {
		servIsInEvent = true;
	}

	if (con->protocolBase != NULL && !servIsInEvent) {
		con->protocolBase->protocol_releaseBackendConnect(*con, type);
		con->protocolBase->destoryInstance();
	}

	if (!servIsInEvent) {
		if (con->database.slaveDataBase != NULL
				&& !con->database.dataBaseGroup->is_slaveUseDataBaseConnectNumToBalace()) {
			con->database.slaveDataBase->dec_allocToFrontConn();
		}

		if (con->database.masterDataBase != NULL
				&& !con->database.dataBaseGroup->is_masterUseDataBaseConnectNumToBalace()) {
			con->database.masterDataBase->dec_allocToFrontConn();
		}

		this->connectList.remove(con);
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
//		this->add_connectFdRelation(con->servns()->get_fd(), con);
		logs(Logger::INFO, "add server fd(%d) to epoll", con->servns()->get_fd());
		this->ioEvent->add_ioEventRead(con->servns()->get_fd(), ClientThread::rw_backendData, con);
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
			} else if (base != NULL) {
				base->destoryInstance();
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

void ClientThread::handle_readBackendData(Connection& conn)
{
	if (conn.sock.curservs == NULL)
		return;
	unsigned int fd = conn.sock.curservs->get_fd();
	logs(Logger::INFO, "handle_readBackendData(%d)", fd);

	conn.activeTime = config()->get_globalSecondTime();
	logs(Logger::DEBUG, "fd: %d, activeTime: %llu", fd, conn.activeTime);

	//2. read data from backend
	NetworkSocket *sns = conn.sock.get_curServSock();
	if (sns == NULL)
		return;

	int ret = sns->read_data();
	if (ret < 0) {
		logs(Logger::WARNING, "read data error, close connection");
		this->finished_connection(&conn, CONN_FINISHED_ERR);
		return;
	} else if (ret == 2) {
		/***
		 * 后端关闭，比如数据库强制关闭的情况。此时不能复用连接
		 * **/
		logs(Logger::WARNING, "backendend force close connection");
		this->finished_connection(&conn, CONN_FINISHED_ERR);
		return;
	}
	if (sns->get_recvData().get_length() == 0) {
		return;
	}

	if (conn.sock.curclins != NULL) {//when in the case that clean backend socket
		logs(Logger::DEBUG, "fd(%d) send (%d) bytes data to front, clientHashCode: %u",
				fd,  sns->get_recvData().get_length(), conn.sock.curclins->get_addressHashCode());
	} else {
		logs(Logger::DEBUG, "fd(%d) send(%d)bytes data to oneproxy", fd, sns->get_recvData().get_length());
	}
	logs_buf((char*)"backend =====> front",(void*)sns->get_recvData().get_buf(), sns->get_recvData().get_length());

	//3. parse backend packet
	int tret = 0;
	if ((tret = this->parse_backendDataPacket(&conn))) {
		if (tret == 1) {
			this->finished_connection(&conn, CONN_FINISHED_NORMAL);
		} else {
			logs(Logger::WARNING, "parse backend data packet error");
			this->finished_connection(&conn, CONN_FINISHED_ERR);
		}
		return ;
	}

	//4. send data
	this->send_data(conn, true);
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
	logs(Logger::DEBUG, "sendToClient:%d", sendToClient);
	if (sendToClient) {
		NetworkSocket* sns = conn.sock.curservs;
		if (sns != NULL) {
			if (sns->get_recvData().get_remailLength() > 0) {
				uif (this->write_data(conn, true)) {
					logs(Logger::ERR, "write data error");
					return -1;
				}
			}
			if (sns->get_sendData().get_remailLength() > 0) {
				uif (this->write_data(conn, false)) {
					logs(Logger::ERR, "write data error");
					return -1;
				}
			}
		}

		NetworkSocket* cns = conn.sock.curclins;
		if (cns != NULL) {
			if (cns->get_sendData().get_remailLength() > 0) {
				uif (this->write_data(conn, true)) {
					logs(Logger::ERR, "write data error");
					return -1;
				}
			}
		}
	} else {
		NetworkSocket* cns = conn.sock.curclins;
		if (cns != NULL) {
			if (cns->get_recvData().get_remailLength() > 0) {
				uif (this->write_data(conn, false)) {
					logs(Logger::ERR, "write data error");
					return -1;
				}
			}

			if (cns->get_sendData().get_remailLength() > 0) {
				uif (this->write_data(conn, true)) {
					logs(Logger::ERR, "write data error");
					return -1;
				}
			}
		}

		//add by huih@20161220
		NetworkSocket* sns = conn.sock.curservs;
		if (sns != NULL) {
			if (sns->get_sendData().get_remailLength() > 0) {
				uif (this->write_data(conn, false)) {
					logs(Logger::ERR, "write data error");
					return -1;
				}
			}
		}
	}

	return 0;
}

int ClientThread::write_data(Connection& con, bool isFront)
{
	int ret = 0;
	NetworkSocket* tSocket = NULL;
	StringBuf* tData = NULL;
	StringBuf emptyPacket;
	Func func = NULL;
	NetworkSocket* sSocket = NULL; //source socket.
	Func sFunc = NULL;

	logs(Logger::DEBUG, "isFront: %d", isFront);
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
		return 0;
	}

#define save_data_toSendData() do{\
	logs(Logger::INFO, "rewrite data ...., fd:%d, data.offset: %d, "\
		"data.length: %d, sendBuf.length: %d", desSocket.get_fd(),\
			data.get_offset(), data.length(), desSocket.get_sendData().get_length());\
	desSocket.save_data(data);\
	this->ioEvent->add_ioEventWrite(desSocket.get_fd(), func, &con);\
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
				logs(Logger::WARNING, "write data error, close connection");
				this->finished_connection(&con, CONN_FINISHED_ERR);
				return -1;
			} else {//write finished.
				sendData.clear();
				if (sSocket != NULL)
					this->ioEvent->add_ioEventRead(sSocket->get_fd(), sFunc, &con);
			}
		}

		ret = desSocket.write_data(data);
		if (ret == 1) {
			save_data_toSendData();
			break;
		} else uif (ret) {
			logs(Logger::WARNING, "write data to (%d) error", desSocket.get_fd());
			this->finished_connection(&con, CONN_FINISHED_ERR);
			return -1;
		} else {//写数据完毕
			logs(Logger::INFO, "write finished ...");
			this->ioEvent->add_ioEventRead(desSocket.get_fd(), func, &con);
			break;
		}
	} while(0);
	data.clear();
	return 0;
}

void ClientThread::add_FailConnQueue(Connection* conn)
{
	TryFailedConnection* tfc = new TryFailedConnection();
	tfc->clientThread = this;
	tfc->connection = conn;

	this->ioEvent->add_timerEvent(10.0, 0.0, ClientThread::retry_failedConnection, tfc);

	taskStat.doing_failConnection();
	record()->record_threadFailConn(this->get_threadId());
}

void ClientThread::check_connectionTimeout() {

	Connection* conn = NULL;
	u_uint64 gst = config()->get_globalSecondTime();
	ConnectionList::iterator it = this->connectList.begin();
	for (; it != this->connectList.end(); ++it) {
		if ((gst > (*it)->activeTime)
				&& (gst - (*it)->activeTime) > (u_uint64)(config()->get_connectTimeOut())) {//one day
			conn = *it;
			break;//one time, close one connection.
		}
	}
	if (conn != NULL) {
		logs(Logger::WARNING, "timeout close client fd: %d, globalSecondTime: %lld",
				conn->sock.curclins->get_fd(), gst);
		this->finished_connection(conn, CONN_FINISHED_ERR);
	}
}

unsigned int ClientThread::get_threadTaskNum()
{
	return taskStat.sum_connection();
}

int ClientThread::get_socketPairReadFd()
{
	return this->spfd[1];
}

int ClientThread::get_socketPairWriteFd()
{
	return this->spfd[0];
}

int ClientThread::write_socketPair(const void* data, const unsigned int dataLen)
{
	uif(SocketUtil::socket_writeData(this->get_socketPairWriteFd(), data, dataLen, 500)) {
		logs(Logger::ERR, "write data error");
		return -1;
	}
	return 0;
}

void ClientThread::rw_frontData(unsigned int fd, unsigned int events, void* args)
{
	Connection* conn = (Connection*)args;
	ClientThread* ct = (ClientThread*)conn->clientThreadObj;
	assert(conn != NULL);
	assert(ct != NULL);

	logs(Logger::INFO, "front fd(%d) have data to read", fd);
	config()->update_globalTime();
	if (ct->ioEvent->is_readEvent(events)) {
		ct->handle_readFrontData(*conn);
	} else if (ct->ioEvent->is_writeEvent(events)) {
		ct->write_data(*conn, true);
	} else {
		logs(Logger::ERR, "unknow events(%d)", events);
	}
}

void ClientThread::rw_backendData(unsigned int fd, unsigned int events, void *args)
{
	Connection* conn = (Connection*)args;
	ClientThread* ct = (ClientThread*)conn->clientThreadObj;
	assert(conn != NULL);
	assert(ct != NULL);

	logs(Logger::INFO, "backend fd(%d) have data to read, events(%d)", fd, events);
	config()->update_globalTime();
	if (ct->ioEvent->is_readEvent(events)) {
		logs(Logger::DEBUG, "is_readEvent");
		ct->handle_readBackendData(*conn);
	} else if (ct->ioEvent->is_writeEvent(events)) {
		logs(Logger::DEBUG, "is_writeEvent");
		ct->write_data(*conn, false);
	} else {
		logs(Logger::ERR, "unknow events(%d)", events);
	}
}

void ClientThread::read_clientSocket(unsigned int fd, unsigned int events, void* args)
{
	ClientThread* ct = (ClientThread*)args;
	StringBuf sb;
	union {
		NetworkSocket* ns;
		long l;
	} d;
	d.l = 0;

	if (SocketUtil::socket_readAllData(ct->get_socketPairReadFd(), sb, 500) < 0) {
		logs(Logger::ERR, "read client connection error");
		return;
	}

	unsigned int pl = sizeof(d.l);
	while(sb.get_offset() < sb.length()) {

		if (sb.get_remailLength() >= pl) {
			memcpy(&d.l, sb.addr() + sb.get_offset(), pl);
			sb.set_offset(sb.get_offset() + pl);
		}

		ct->taskStat.recv_connection();
		record()->record_threadRecvConn(ct->get_threadId());

		ct->add_task(d.ns);
	}
}

void ClientThread::check_connectionTimeout(void* args)
{
	ClientThread* ct = (ClientThread*)args;
	ct->check_connectionTimeout();
}

void ClientThread::retry_failedConnection(void* args)
{
	TryFailedConnection* tfc = (TryFailedConnection*)args;
	if(!tfc || !tfc->clientThread || !tfc->connection) {
		return;
	}
	ClientThread* ct = (ClientThread*)tfc->clientThread;
	Connection* conn = (Connection*)tfc->connection;
	ct->taskStat.retry_failConnection();
	ct->handle_readFrontData(*conn);
}

thread_start_func(ClientThread::start)
{
//	ProfilerRegisterThread();
	ClientThread *ct = (ClientThread*)args;
	uif (ct == NULL)
		return 0;
	//add socket pair read end to epoll.
	ct->ioEvent->add_ioEventRead(ct->get_socketPairReadFd(), ClientThread::read_clientSocket, ct);
	//check connection is timeout or not
	ct->ioEvent->add_timerEvent(10.0, 10.0, ClientThread::check_connectionTimeout, ct);
	ct->ioEvent->regester_checkQuit();
	ct->ioEvent->run_loop();

	return 0;
}
