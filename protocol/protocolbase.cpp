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
* @FileName: protocolbase.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月12日
*
*/

#include "protocolbase.h"
#include "connectionpool.h"
#include <sstream>
#include <iostream>
using namespace std;

ProtocolHandleRetVal ProtocolBase::protocol_front(Connection& conn)
{
	StringBuf& packet = conn.sock.get_clientSock()->get_recvData();
	unsigned int oldOffset = packet.get_offset();
	ProtocolHandleRetVal resultValue = HANDLE_RETURN_SUCCESS;

	do {
		//1. 统计前端接收到的数据长度
		this->stat_readFrontData(conn);

		//2. 预处理，不同的数据库的数据使用不同的逻辑
		//sqlserver中把前端数据进行合并，直到得到一个完整的数据包为止。
		resultValue = this->prehandle_frontPacket(conn);
		if (resultValue != HANDLE_RETURN_SUCCESS) {//可能出错或者需要直接转发
			uif (resultValue == HANDLE_RETURN_ERROR) {
				logs(Logger::ERR, "prehandle front packet error");
			}
			break;
		}

		//3.预处理成功，则继续处理此包，执行包头部的处理函数
		if (this->protocol_initFrontStatPacket(conn)) {
			logs(Logger::ERR, "handle front packet header error");
			resultValue = HANDLE_RETURN_ERROR;
			break;
		}

		//4. 调用注册函数进行处理
		StringBuf& desPacket = *(conn.sock.get_clientSock()->get_bufPointer());
		for(; desPacket.get_offset() < desPacket.length(); ) {//循环处理，直到所有的数据处理完毕
			int type = this->get_packetType(desPacket);
			ProtocolBase::BaseHandleFuncMap::iterator it = this->frontHandleFunc.find(type);
			lif(it != this->frontHandleFunc.end()) {
				resultValue = (this->*it->second)(conn, desPacket);
				if (resultValue == HANDLE_RETURN_SEND_TO_CLIENT) {
					//1. 把despacket包中的数据复制到发送包中
					StringBuf& sendPacket = conn.clins()->get_sendData();
					sendPacket.append((void*)(desPacket.addr() + desPacket.get_offset()), desPacket.get_remailLength());

					//2. 把客户端接收包中的数据清理掉，防止发送到服务端,同时清理掉despacket的数据
					conn.clins()->get_recvData().clear();
					desPacket.clear();
					break;
				} else if (resultValue != HANDLE_RETURN_SUCCESS) {
					break;
				}
			} else {
				logs(Logger::DEBUG, "not find front handle function, packet type is %d, offset: %d",
						(int)type, desPacket.get_offset());
				logs_buf_force("desPacket", desPacket.addr(), desPacket.length());
				logs_buf_force("desPacket+offset", (char*)(desPacket.addr() + desPacket.get_offset()),
						desPacket.length() - desPacket.get_offset());
				break;
			}
		}

		//5. 清理
		this->protocol_endFrontStatPacket(conn);
	}while(0);

	if (packet.length() > oldOffset)//when clear, the oldOffset maybe bigger than length.
		packet.set_offset(oldOffset);

	return resultValue;
}

ProtocolHandleRetVal ProtocolBase::protocol_backend(Connection& conn)
{
	StringBuf& packet = conn.sock.get_curServSock()->get_recvData();
	int oldOffset = packet.get_offset();
	ProtocolHandleRetVal resultValue = HANDLE_RETURN_SUCCESS;
	do {
		//1. 累计后端得到的数据
		this->stat_readBackendData(conn);

		//2. 预处理
		resultValue = this->prehandle_backendPacket(conn);
		if (resultValue != HANDLE_RETURN_SUCCESS) {//可能出现错误，或者需要直接转发
			break;
		}

		//3.预处理成功，则继续处理此包，执行包头部的处理函数
		if (this->protocol_initBackendStatPacket(conn)) {
			logs(Logger::ERR, "handle front packet header error");
			resultValue = HANDLE_RETURN_ERROR;
			break;
		}

		//4. 调用注册函数进行处理
		StringBuf& desPacket = *(conn.sock.get_curServSock()->get_bufPointer());
		for(; desPacket.get_offset() < desPacket.length(); ) {//循环处理，直到所有的数据处理完毕
			int type = this->get_packetType(desPacket);
			ProtocolBase::BaseHandleFuncMap::iterator it = this->backendHandleFunc.find(type);
			lif(it != this->backendHandleFunc.end()) {
				resultValue = (this->*it->second)(conn, desPacket);
				if (resultValue != HANDLE_RETURN_SUCCESS) {
					break;
				}
			} else {
				logs(Logger::DEBUG, "not find backend packet handle function, packet type is %d, offset: %d",
						(int)type, desPacket.get_offset());
//				logs_buf_force("desPacket", desPacket.addr(), desPacket.length());
//				logs_buf_force("desPacket+offset", (char*)(desPacket.addr() + desPacket.get_offset()),
//										desPacket.length() - desPacket.get_offset());
				resultValue = HANDLE_RETURN_ERROR;
				break;
			}
		}

		//5. 清理
		this->protocol_endBackendStatPacket(conn);
	} while(0);

	packet.set_offset(oldOffset);

	return resultValue;
}

int ProtocolBase::protocol_getBackendConnect(Connection& conn)
{
	//1. choose database
	this->protocol_chooseDatabase(conn);

	//2. create socket.
	return this->protocol_createBackendConnect(conn);
}

int ProtocolBase::protocol_releaseBackendConnect(Connection& conn, ConnFinishType type)
{
	if (conn.database.dataBaseGroup && conn.database.dataBaseGroup->get_useConnectionPool()) {

		//when has error in connection, release the server connection.
		if (conn.status == CONN_EXEC_STATUS_ERROR) {
			type = CONN_FINISHED_ERR;
		}

		if (conn.record.type >= TRANS_QUERY_TYPE && conn.record.type < TRANS_QUERY_FINISHED_TYPE) {
			logs(Logger::WARNING, "current connection in trans error");//有可能是前端主动断开导致
			type = CONN_FINISHED_ERR;
		}

		if (type != CONN_FINISHED_ERR) {
			uif ( conn.sock.masters != NULL && this->set_oldSocketToPool(conn.sock.masters, type)) {
				type = CONN_FINISHED_ERR;
			} else{
				conn.sock.masters = NULL;
			}

			uif (conn.sock.slavers != NULL && this->set_oldSocketToPool(conn.sock.slavers, type)) {
				type = CONN_FINISHED_ERR;
			} else {
				conn.sock.slavers = NULL;
			}
		}

		if (type == CONN_FINISHED_ERR) {
			if(conn.sock.masters != NULL) {
				ConnectionPool::get_pool().release_backendSocket(conn.sock.masters);
			}
			if (conn.sock.slavers != NULL) {
				ConnectionPool::get_pool().release_backendSocket(conn.sock.slavers);
			}
		}
	} else {
		if (conn.sock.masters != NULL) {
			delete conn.sock.masters;
		}
		if (conn.sock.slavers != NULL) {
			delete conn.sock.slavers;
		}
	}

	conn.sock.masters = NULL;
	conn.sock.slavers = NULL;
	return 0;
}

int ProtocolBase::protocol_chooseDatabase(Connection& conn)
{
	if (conn.database.dataBaseGroup && conn.database.dataBaseGroup->get_readSlave()){
		if ((conn.record.type == SIMPLE_QUERY_SELECT_TYPE) && (conn.database.slaveDataBase != NULL)) {
			conn.database.currentDataBase = conn.database.slaveDataBase;
		} else {
			conn.database.currentDataBase = conn.database.masterDataBase;
		}
	} else {
		conn.database.currentDataBase = conn.database.masterDataBase;
	}

	if (conn.database.currentDataBase == conn.database.masterDataBase) {
		conn.sock.curservs = conn.sock.masters;
		logs(Logger::DEBUG, "current use master database");
	} else {
		logs(Logger::DEBUG, "current use slave database");
		conn.sock.curservs = conn.sock.slavers;
	}
	return 0;
}

int ProtocolBase::protocol_createBackendConnect(Connection& conn)
{
	assert(conn.curdb() != NULL);
	NetworkSocket* ns = NULL;
	if (conn.servns() == NULL) {

		//1. 使用连接池
		if (conn.database.dataBaseGroup && conn.database.dataBaseGroup->get_useConnectionPool()) {
			NetworkSocket* cns = conn.sock.curclins;
			std::string connArgs = std::string("");
			if (cns) connArgs = cns->connArgsMap2String();
			ns = ConnectionPool::get_pool().get_backendConnect(
					conn.curdb()->get_addr(), conn.curdb()->get_port(), connArgs);
			conn.servns() = ns;
		}

		//2.从后端创建连接
		if (ns == NULL) {
			ns = new NetworkSocket(conn.curdb()->get_addr(), conn.curdb()->get_port());
			logs(Logger::DEBUG, "server address: %s, port: %d", ns->get_address().c_str(), ns->get_port());
			conn.servns() = ns;

			if (conn.curdb()->get_connectNum() >= conn.curdb()->get_connectNumLimit()) {
				logs(Logger::ERR, "current connect num(%d) > connect num limit(%d)",
						conn.curdb()->get_connectNum(),
						conn.curdb()->get_connectNumLimit());
				return -1;
			}

			if (tcpClient.get_backendConnection(ns)
					|| this->protocol_initBackendConnect(conn)) {
				logs(Logger::ERR, "connection to backend error(address: %s, port:%d)",
						ns->get_address().c_str(), ns->get_port());
				return -1;
			}
			ns->set_dataBase(conn.curdb());
			conn.curdb()->inc_connectionNum();

			logs(Logger::DEBUG, "Create new socket(%d)", conn.sock.curservs->get_fd());
			if (conn.database.dataBaseGroup && conn.database.dataBaseGroup->get_useConnectionPool()) {
				this->add_socketToPool(conn.servns());
			}
		}

		if (conn.curdb() == conn.database.masterDataBase) {
			logs(Logger::DEBUG, "current use master database");
			conn.sock.masters = ns;
		} else if (conn.curdb() == conn.database.slaveDataBase){
			logs(Logger::DEBUG, "current use slave database");
			conn.sock.slavers = ns;
		} else {
			logs(Logger::ERR, "unkown database type, addr: %s, port: %d",
					conn.curdb()->get_addr().c_str(), conn.curdb()->get_port());
		}
		logs(Logger::DEBUG, "server fd: %d", ns->get_fd());
	}
	return 0;
}

bool ProtocolBase::is_frontPacket(int type)
{
	BaseHandleFuncMap::iterator it = frontHandleFunc.find(type);
	if (it != frontHandleFunc.end())
		return true;
	return false;
}

void ProtocolBase::regester_frontHandleFunc(int type, BaseFunc func)
{
	this->frontHandleFunc[type] = func;
}

void ProtocolBase::regester_backendHandleFunc(int type, BaseFunc func)
{
	this->backendHandleFunc[type] = func;
}

implement_protocol_handle_func(ProtocolBase, handle_defaultPacket)
{
	return HANDLE_RETURN_SEND_DIRECT;//不进行后续处理，直接把数据转发到对端。
}

ProtocolHandleRetVal ProtocolBase::prehandle_frontPacket(Connection& conn)
{
	//默认实现，直接执行后续操作
	return HANDLE_RETURN_SUCCESS;
}

ProtocolHandleRetVal ProtocolBase::prehandle_backendPacket(Connection& conn)
{
	//默认实现，直接执行后续操作。
	return HANDLE_RETURN_SUCCESS;
}

int ProtocolBase::protocol_initFrontStatPacket(Connection& conn)
{
	assert(conn.clins() != NULL);

	//默认情况下，直接设置需要处理的数据为接收到的数据包
	NetworkSocket* ns = conn.clins();
	ns->set_bufPointer(&(ns->get_recvData()));
	return 0;
}

int ProtocolBase::protocol_initBackendStatPacket(Connection& conn)
{
	assert(conn.sock.curservs != NULL);
	NetworkSocket* ns = conn.sock.curservs;
	ns->set_bufPointer(&(ns->get_recvData()));
	//默认情况下，直接设置需要处理的数据为接收到的后端的数据包
	return 0;
}

int ProtocolBase::protocol_endFrontStatPacket(Connection& conn)
{
	return 0;
}

int ProtocolBase::protocol_endBackendStatPacket(Connection& conn)
{
	return 0;
}

//返回sqlParser对象
SqlParser& ProtocolBase::get_sqlParser()
{
	return this->sqlParser;
}

unsigned int ProtocolBase::get_currentSqlHashCode(Connection& conn)
{
	return conn.record.sqlInfo.sqlHashCode;
}

std::string& ProtocolBase::get_currentSqlText(Connection& conn)
{
	return conn.record.sqlInfo.sqlText;
}

//此函数中调用parse函数后，并且通过解析器修改sql语句，通过modifySqlText返回替换sql语句的中的值为?后的sql语句。
int ProtocolBase::parse_sql(Connection& conn, std::string sqlText)
{
	if(this->sqlParser.parse(sqlText.c_str()) == false) {
		logs(Logger::ERR, "sql parser parse sql error");
		return -1;
	}

	this->sqlParser.toPatternQuery(conn.record.sqlInfo.sqlText);

	conn.record.sqlInfo.queryType = (stats::SqlOp)this->sqlParser.queryType();
	conn.record.sqlInfo.tableCount = this->sqlParser.tableCount();

	//save table name
	conn.record.sqlInfo.tableNameVec.clear();
	for (unsigned int i = 0; i < conn.record.sqlInfo.tableCount; ++i) {
		const char* tmpTable = this->sqlParser.table(i)->table;
		conn.record.sqlInfo.tableNameVec.push_back(std::string(tmpTable, strlen(tmpTable)));
	}

	if (this->sqlParser.isStartTrans()) {
		logs(Logger::INFO, "start trans");
		conn.record.type = TRANS_QUERY_BEGIN_TYPE;
	} else if (this->sqlParser.isCommit()) {
		conn.record.type = TRANS_QUERY_COMMIT_TYPE;
		logs(Logger::INFO, "COMMIT TRANS");
	} else if (this->sqlParser.isRollBack()) {
		conn.record.type = TRANS_QUERY_ROLLBACK_TYPE;
		logs(Logger::INFO, "ROLLBACK trans");
	}

	return 0;
}

//统计读取到前端的数据
void ProtocolBase::stat_readFrontData(Connection& conn)
{
	record()->record_clientQuerySendSize(conn, conn.clins()->get_addressHashCode(),
			conn.clins()->get_recvData().get_length(), conn.sessData.clientInfo);
}

//统计从后端返回的数据
void ProtocolBase::stat_readBackendData(Connection& conn)
{
	lif (conn.clins() != NULL && conn.servns() != NULL) {
		record()->record_clientQueryRecvSize(conn, conn.clins()->get_addressHashCode(),
				conn.servns()->get_recvData().get_length(), conn.sessData.clientInfo);
	}

	lif (conn.servns() != NULL) {
		record()->record_sqlInfoRecvSize(conn, conn.record.sqlInfo.sqlHashCode,
				conn.servns()->get_recvData().get_length(), conn.sessData.sqlInfo);
	}
}

//开始事务
void ProtocolBase::stat_startTrans(Connection& conn)
{
	//record translation start data
	logs(Logger::INFO, "stat start trans");
	conn.record.trans_start_time = conn.record.startQueryTime;
	conn.record.sqlSet.insert(conn.record.sqlInfo.sqlHashCode);
	conn.record.type = TRANS_QUERY_TYPE;
}

//结束事务
void ProtocolBase::stat_endTrans(Connection& conn, bool isRollBack)
{
	logs(Logger::INFO, "stat end trans");
	do{
		if (isRollBack) {
			conn.record.rollback = true;
		}

		std::string hashCodeKeys;
		{
			std::set<unsigned int>::iterator it = conn.record.sqlSet.begin();
			for(; it != conn.record.sqlSet.end(); ++it) {
				if (hashCodeKeys.size() > 0) {
					hashCodeKeys.append(";");
				}
				hashCodeKeys.append(Tool::itoa(*it));
			}
		}

		if (hashCodeKeys.size() <= 0)
			break;

		u_uint64 transHashCode = Tool::quick_hash_code(hashCodeKeys.c_str(), hashCodeKeys.length());
		record()->record_transInfoEndTrans(transHashCode, conn);
		conn.record.sqlSet.clear();
	}while(0);
	conn.record.type = QUERY_TYPE_INIT;
}

void ProtocolBase::stat_saveSql(Connection& conn, std::string& sqlText)
{
	conn.record.sqlInfo.sqlText = sqlText;
	conn.record.sqlInfo.sqlHashCode = Tool::quick_hash_code(sqlText.c_str(), sqlText.length());

	record()->record_sqlInfoAddSql(conn);
	record()->record_clientQueryAddSql(conn, conn.sessData.clientInfo);
}

//记录当前执行的sql，这个sql是经过解析器改写后的sql语句，及把值替换后的sql语句
void ProtocolBase::stat_executeSql(Connection& conn, std::string& sqlText)
{
	//把解析后的sqlText记录到record
	stat_saveSql(conn, sqlText);

	//执行sql
	stat_executeSql(conn);
}

int ProtocolBase::stat_parseSql(Connection& conn, std::string& sqlText)
{
	logs_logsql("%s", sqlText.c_str());
	if (sqlText.length() <= 0) {
		logs(Logger::INFO, "sqltext is empty");
		return 0;
	}

	//1. 解析sql语句
	if (this->parse_sql(conn, sqlText)) {
		return -1;
	}

	//2. record
	this->stat_saveSql(conn, conn.record.sqlInfo.sqlText);
	return 0;
}

//根据connection中的hashcode来统计执行的sql语句
void ProtocolBase::stat_executeSql(Connection& conn)
{
	//record time
	conn.record.startQueryTime = config()->get_globalMillisecondTime();
	conn.record.totalRow = 0;

	if (conn.record.sqlInfo.sqlHashCode <= 0)
		return;

	//record client
	record()->record_clientQueryExec(conn.record.sqlInfo.sqlHashCode,
			conn.clins()->get_addressHashCode(), conn.sessData.clientInfo);

	record()->record_sqlInfoExec(conn.record.sqlInfo.sqlHashCode, conn.sessData.sqlInfo);
	//in trans.
	if (conn.record.type >= TRANS_QUERY_TYPE && conn.record.type < TRANS_QUERY_SUM) {
		conn.record.sqlSet.insert(conn.record.sqlInfo.sqlHashCode);
		record()->record_sqlInfoExecTran(conn, conn.record.sqlInfo.sqlHashCode, conn.sessData.sqlInfo);
	}
}

int ProtocolBase::stat_parseAndStatSql(Connection& conn, std::string sqlText)
{
	logs_logsql("%s", sqlText.c_str());
	conn.record.sqlInfo.sqlText.clear();

	//1. 解析sql语句
	if (this->parse_sql(conn, sqlText)) {
		return -1;
	}

	//2. 统计sql语句
	this->stat_executeSql(conn, conn.record.sqlInfo.sqlText);

	return 0;
}

int ProtocolBase::stat_preparedSql(Connection& conn, unsigned int preparedHandle)
{
	//find sqlHashCode;
	BackendHandle backendHandle;
	uif (conn.handleManager().get_backendHandleBasePrepared(preparedHandle, backendHandle)) {
		logs(Logger::ERR, "preparedHandle(%u) no sql", preparedHandle);
		return -1;
	}
	conn.record.sqlInfo.sqlHashCode = backendHandle.hashCode;
	if (conn.record.sqlInfo.sqlHashCode <= 0){
		logs(Logger::ERR, "sql hash code is zero, preparedHandle: %u", preparedHandle);
		return -1;
	}

	stats::SqlInfo* sqlInfo = record()->find_sqlInfo(conn.record.sqlInfo.sqlHashCode);
	if (sqlInfo != NULL) {
		conn.record.sqlInfo.sqlText = sqlInfo->sqlText;
		logs_logsql("prepare sql: %s, preparedHandle: %u", sqlInfo->sqlText.c_str(), preparedHandle);
	}

	this->stat_executeSql(conn);

	return 0;
}

//统计查询结果
//接收到一行数据
void ProtocolBase::stat_recvOneRow(Connection& conn)
{
	conn.record.totalRow ++;
}

//接收完成，其中rows时结束包中提供的总数据行数，如果没有，可以不用填写
void ProtocolBase::stat_recvFinishedRow(Connection& conn, unsigned int rows)
{
	record()->record_sqlInfoRecvResult(conn, rows, conn.sessData.sqlInfo);
	conn.record.totalRow = 0;
}

//sql语句执行错误,这个函数应该与stat_recvFinishedRow互斥
void ProtocolBase::stat_executeErr(Connection& conn)
{
	record()->record_sqlInfoExecFail(conn, conn.sessData.sqlInfo);
	record()->record_clientQueryExecFail(conn, conn.sessData.clientInfo);
}

void ProtocolBase::stat_login(Connection& conn)
{
}

void ProtocolBase::add_socketToPool(NetworkSocket* ns)
{
	ConnectionPool::get_pool().save_backendConnect(ns,
			&NetworkSocket::destroy_networkSocket, &ProtocolBase::check_socketActive, true);
}

int ProtocolBase::set_oldSocketToPool(NetworkSocket* ns, ConnFinishType type)
{
	if (ns != NULL)
		return ConnectionPool::get_pool().set_backendConnect(ns);
	return -1;
}

std::string ProtocolBase::get_sqlText(unsigned int hashCode)
{
	std::string sqlText;
	stats::SqlInfo* sqlInfo = record()->find_sqlInfo(hashCode);
	if (sqlInfo != NULL)
		sqlText = sqlInfo->sqlText;
	return sqlText;
}

void ProtocolBase::mark_sqlParamPosition(PreparedDataInfoT& preparedDataInfot)
{
	if (preparedDataInfot.sql.length() > 0 && preparedDataInfot.paramNum > 0)
	{
		for(int i = 0; i < (int)preparedDataInfot.paramNum; ++i) {
			int startPos = preparedDataInfot.sql.find(preparedDataInfot.param[i].paramName);
			if (startPos != (int)preparedDataInfot.sql.npos) {
				preparedDataInfot.param[i].paramStartPos = startPos;
				preparedDataInfot.param[i].paramEndPos = preparedDataInfot.param[i].paramStartPos
						+ preparedDataInfot.param[i].paramName.length();
			} else {
				preparedDataInfot.param[i].paramStartPos = 0;
				preparedDataInfot.param[i].paramEndPos = 0;
			}
		}
	}
}

void ProtocolBase::fill_originSqlValue(const PreparedDataInfoT& preparedDataInfot, Connection& conn)
{
	if (preparedDataInfot.paramNum <= 0 || preparedDataInfot.sql.length() <= 0) {
		if (preparedDataInfot.sql.length() > 0) {
			output_originSql(preparedDataInfot.sql, conn);
		}
		return;
	}

	if (preparedDataInfot.paramNum != preparedDataInfot.paramValueVec.size()) {
		logs(Logger::ERR, "The number of the parameter value is not equal the number of param error");
		logs(Logger::ERR, "paramNum: %d, paramValueNum:%d", preparedDataInfot.paramNum,
				preparedDataInfot.paramValueVec.size());
		output_originSql(preparedDataInfot.sql, conn);//如果参数数量和参数值不一致，只输出原始的sql语句
		return;
	}

	/**
	 * 为了防止在填充sql语句时导致位置的变动，从最后一个参数向第一个参数来填充值
	 * **/
	std::string tmpsql = (std::string)preparedDataInfot.sql;
	std::string value;
	for (int i = (int)(preparedDataInfot.paramNum - 1); i >= 0; --i) {
		PreparedParamT& ppt = (PreparedParamT&)preparedDataInfot.param[i];

		if (ppt.paramStartPos == 0 || ppt.paramEndPos == 0) {
			continue;
		}
		value = preparedDataInfot.paramValueVec.at(i);
		if (ppt.paramType == PARAM_TYPE_STRING) {//需要按照字符串处理
			std::stringstream ss;
			ss << std::string(tmpsql.c_str(), ppt.paramStartPos);
			ss << "\"";
			ss << value;
			ss << "\"";
			ss << std::string((char*)(tmpsql.c_str() + ppt.paramEndPos), tmpsql.length() - ppt.paramEndPos);
			tmpsql = ss.str();
		} else {//直接替换参数即可
			std::stringstream ss;
			ss << std::string(tmpsql.c_str(), ppt.paramStartPos);
			ss << value;
			ss << std::string((char*)(tmpsql.c_str() + ppt.paramEndPos), tmpsql.length() - ppt.paramEndPos);
			tmpsql = ss.str();
		}
	}
	output_originSql(tmpsql, conn);
}

void ProtocolBase::output_originSql(const std::string& sql, Connection& conn)
{
	if (conn.sock.curclins) {
		NetworkSocket* client = conn.sock.curclins;
		std::stringstream ss;
		ss << "[";
		ss << client->get_address();
		ss << ":";
		ss << client->get_port();
		ss << "]";
		if (conn.sessData.hostName.length() > 0) {
			ss << "[";
			ss << conn.sessData.hostName;
			ss << "]";
		}
		if (conn.sessData.appName.length() > 0) {
			ss << "[";
			ss << conn.sessData.appName;
			ss << "] ";
		}
		ss << sql;
		logs_logsql_force("%s", ss.str().c_str());
	} else {
		logs_logsql_force("%s", sql.c_str());
	}
}
