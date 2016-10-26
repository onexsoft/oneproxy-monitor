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

void ProtocolBase::protocol_front(Connection& conn)
{
	StringBuf& packet = conn.sock.get_clientSock()->get_recvData();
	unsigned int oldOffset = packet.get_offset();

	do {
		//1. 统计前端接收到的数据长度
		this->stat_readFrontData(conn);

		//2. 预处理
		PreHandleRetValue ret = this->prehandle_frontPacket(conn);
		if (ret == PREHANDLE_FAILE || ret == PREHANDLE_FORWARD_DIRECT) {//直接转发此包
			break;
		} else if (ret != PREHANDLE_SUCCESS) {//
			logs(Logger::ERR, "unkown return code(%d)", ret);
			break;
		}

		//3.预处理成功，则继续处理此包，执行包头部的处理函数
		if (this->protocol_initFrontStatPacket(conn)) {
			logs(Logger::ERR, "handle front packet header error");
			break;
		}

		//4. 调用注册函数进行处理
		StringBuf& desPacket = *(conn.sock.get_clientSock()->get_bufPointer());
		for(; desPacket.get_offset() < desPacket.length(); ) {//循环处理，直到所有的数据处理完毕
			int type = this->get_packetType(desPacket);
			ProtocolBase::BaseHandleFuncMap::iterator it = this->frontHandleFunc.find(type);
			lif(it != this->frontHandleFunc.end()) {
				ProtocolHandleRetVal hret = (this->*it->second)(conn, desPacket);
				if(hret == HANDLE_RETURN_ERROR) {//协议成没有找到对应的类型或者其他的情况
					logs(Logger::INFO, "handle front packet error or need send direct, packet type is %d", (int)type);
					break;
				} else if (hret == HANDLE_RETURN_SEND_DIRECT) {
					break;
				} else if (hret == HANDLE_RETURN_SEND_TO_CLIENT) {
					//1. 把despacket包中的数据复制到发送包中
					StringBuf& sendPacket = conn.clins()->get_sendData();
					sendPacket.append((void*)(desPacket.addr() + desPacket.get_offset()), desPacket.get_remailLength());

					//2. 把客户端接收包中的数据清理掉，防止发送到服务端
					conn.clins()->get_recvData().clear();
					break;
				}
			} else {
				logs(Logger::ERR, "not find front handle function, packet type is %d, offset: %d",
						(int)type, desPacket.get_offset());
				logs_buf_force("desPacket", desPacket.addr(), desPacket.length());
				logs_buf_force("desPacketxxxx", (char*)(desPacket.addr() + desPacket.get_offset()),
						desPacket.length() - desPacket.get_offset());
				break;
			}
		}

		//5. 清理
		this->protocol_clearFrontStatPacket(conn);
	}while(0);

	if (packet.length() > oldOffset)//when clear, the oldOffset maybe bigger than length.
		packet.set_offset(oldOffset);
}

void ProtocolBase::protocol_backend(Connection& conn)
{
	StringBuf& packet = conn.sock.get_curServSock()->get_recvData();
	int oldOffset = packet.get_offset();

	do{
		//1. 累计后端得到的数据
		this->stat_readBackendData(conn);

		//2. 预处理
		PreHandleRetValue ret = this->prehandle_backendPacket(conn);
		if (ret == PREHANDLE_FAILE) {//直接转发此包
			break;
		} else if (ret == PREHANDLE_FORWARD_DIRECT){
			break;
		} else if (ret != PREHANDLE_SUCCESS) {//
			logs(Logger::ERR, "unkown return code(%d)", ret);
			break;
		}

		//3.预处理成功，则继续处理此包，执行包头部的处理函数
		if (this->protocol_initBackendStatPacket(conn)) {
			logs(Logger::ERR, "handle front packet header error");
			break;
		}

		//4. 调用注册函数进行处理
		StringBuf& desPacket = *(conn.sock.get_curServSock()->get_bufPointer());
		for(; desPacket.get_offset() < desPacket.length(); ) {//循环处理，直到所有的数据处理完毕
			int type = this->get_packetType(desPacket);
			ProtocolBase::BaseHandleFuncMap::iterator it = this->backendHandleFunc.find(type);
			lif(it != this->backendHandleFunc.end()) {
				ProtocolHandleRetVal hret = (this->*it->second)(conn, desPacket);
				if(hret == HANDLE_RETURN_ERROR) {
					logs(Logger::INFO, "handle backend packet error or need send direct, packet type is %d", (int)type);
					break;
				} else if (hret == HANDLE_RETURN_SEND_DIRECT) {
					break;
				}
			} else {
//				logs(Logger::ERR, "not find backend packet handle function, packet type is %d", (int)type);
//				logs_buf_force("desPacket", desPacket.addr(), desPacket.length());
				break;
			}
		}

		//5. 清理
		this->protocol_clearBackendStatPacket(conn);
	} while(0);

	packet.set_offset(oldOffset);
}

int ProtocolBase::protocol_getBackendConnect(Connection& conn)
{
	//1. choose database
	this->protocol_chooseDatabase(conn);

	//2. create socket.
	return this->protocol_createBackendConnect(conn);
}

int ProtocolBase::protocol_chooseDatabase(Connection& conn)
{
	if ((conn.record.type == SIMPLE_QUERY_SELECT_TYPE) && (conn.database.slaveDataBase != NULL)) {
		conn.database.currentDataBase = conn.database.slaveDataBase;
	} else {
		conn.database.currentDataBase = conn.database.masterDataBase;
	}

	if (conn.database.currentDataBase == conn.database.masterDataBase) {
		conn.sock.curservs = conn.sock.masters;
	} else {
		conn.sock.curservs = conn.sock.slavers;
	}
	return 0;
}

int ProtocolBase::protocol_createBackendConnect(Connection& conn)
{
	assert(conn.curdb() != NULL);
	NetworkSocket* ns = NULL;
	if (conn.servns() == NULL) {
		ns = new NetworkSocket(conn.curdb()->get_addr(), conn.curdb()->get_port());
		conn.servns() = ns;
		if (tcpClient.get_backendConnection(ns)
				|| this->protocol_initBackendConnect(conn)) {
			logs(Logger::ERR, "connection to backend error(address: %s, port:%d)",
					ns->get_address().c_str(), ns->get_port());
			return -1;
		}
		if (conn.curdb() == conn.database.masterDataBase) {
			logs(Logger::INFO, "current use master database");
			conn.sock.masters = ns;
		} else if (conn.curdb() == conn.database.slaveDataBase){
			logs(Logger::INFO, "current use slave database");
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

PreHandleRetValue ProtocolBase::prehandle_frontPacket(Connection& conn)
{
	//默认实现，直接执行后续操作
	return PREHANDLE_SUCCESS;
}

PreHandleRetValue ProtocolBase::prehandle_backendPacket(Connection& conn)
{
	//默认实现，直接执行后续操作。
	return PREHANDLE_SUCCESS;
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

int ProtocolBase::protocol_clearFrontStatPacket(Connection& conn)
{
	return 0;
}

int ProtocolBase::protocol_clearBackendStatPacket(Connection& conn)
{
	return 0;
}
//保存prepared handle与sqlhashcode的关系
void ProtocolBase::save_preparedSqlHashCode(Connection& conn,
		unsigned int preparedHandle, unsigned int sqlHashCode)
{
	logs(Logger::INFO, "preparedHandle: %u, sqlHashCode: %u", preparedHandle, sqlHashCode);
	conn.sessData.preparedHandleMap[preparedHandle] = sqlHashCode;
}

unsigned int ProtocolBase::find_preparedSqlHashCode(Connection& conn, unsigned int preparedHandle)
{
	std::map<unsigned int, unsigned int>::iterator it = conn.sessData.preparedHandleMap.find(preparedHandle);
	if (it == conn.sessData.preparedHandleMap.end()){
		logs(Logger::ERR, "not find sql hash code, preparedHandle: %u", preparedHandle);
		return (unsigned int)-1;
	}
	else {
		return it->second;
	}
}

void ProtocolBase::remove_preparedSqlHashCode(Connection& conn, unsigned int preparedHandle)
{
	conn.sessData.preparedHandleMap.erase(preparedHandle);
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
	record()->clientQueryMap[conn.clins()->get_addressHashCode()].part.upDataSize
			+= conn.clins()->get_recvData().get_length();
}

//统计从后端返回的数据
void ProtocolBase::stat_readBackendData(Connection& conn)
{
	record()->clientQueryMap[conn.clins()->get_addressHashCode()].part.downDataSize
			+= conn.servns()->get_recvData().get_length();
	record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode].part.recvSize
			+= conn.servns()->get_recvData().get_length();
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
		u_uint64 finishedTime = SystemApi::system_millisecond();

		Record::TransInfo& info = record()->transInfoMap[transHashCode];
		info.part.transHashCode = transHashCode;
		info.lastTime = conn.record.trans_start_time;
		info.part.exec ++;
		info.part.lastTime = finishedTime - conn.record.trans_start_time;
		if (info.part.lastTime > info.part.maxTime) {
			info.part.maxTime = info.part.lastTime;
		}
		if (info.part.lastTime < info.part.minTime) {
			info.part.minTime = info.part.lastTime;
		}
		info.part.sqlNum = conn.record.sqlSet.size();
		info.part.totalTime += info.part.lastTime;

		if (conn.record.rollback) {
			info.part.rollbackTimes++;
		}

		{
			std::set<unsigned int>::iterator it = conn.record.sqlSet.begin();
			for(; it != conn.record.sqlSet.end(); ++it) {
				info.sqlHashCode.insert(*it);
			}
		}
		conn.record.sqlSet.clear();
	}while(0);
	conn.record.type = QUERY_TYPE_INIT;
}

void ProtocolBase::stat_saveSql(Connection& conn, std::string& sqlText)
{
	conn.record.sqlInfo.sqlText = sqlText;
	conn.record.sqlInfo.sqlHashCode = Tool::quick_hash_code(sqlText.c_str(), sqlText.length());

	Record::SqlInfo& sqlInfo = record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode];
	if (sqlInfo.part.hashCode == 0) {
		sqlInfo.part.hashCode = conn.record.sqlInfo.sqlHashCode;
		sqlInfo.sqlText = conn.record.sqlInfo.sqlText;
		sqlInfo.part.tabs = conn.record.sqlInfo.tableCount;
		sqlInfo.part.type =  (Record::SqlOp)conn.record.sqlInfo.queryType;
		record()->clientQueryMap[conn.clins()->get_addressHashCode()].add_sqlList(sqlInfo.part.hashCode);
		sqlInfo.add_clientSet(conn.clins()->get_addressHashCode());

		//set table info
		for (unsigned int i = 0; i < sqlInfo.part.tabs; ++i) {
			sqlInfo.add_tableSet(conn.record.sqlInfo.tableNameVec.at(i));
		}
	}
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
	conn.record.startQueryTime = SystemApi::system_millisecond();
	conn.record.totalRow = 0;

	if (conn.record.sqlInfo.sqlHashCode <= 0)
		return;

	Record::SqlInfo& sqlInfo = record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode];
	sqlInfo.part.exec++;

	//record client
	record()->clientQueryMap[conn.clins()->get_addressHashCode()].part.queryNum++;
	if (sqlInfo.part.type == stats::sql_op_select) {
		record()->clientQueryMap[conn.clins()->get_addressHashCode()].part.selectNum++;
	} else if (sqlInfo.part.type == stats::sql_op_insert) {
		record()->clientQueryMap[conn.clins()->get_addressHashCode()].part.insertNum++;
	} else if (sqlInfo.part.type == stats::sql_op_update) {
		record()->clientQueryMap[conn.clins()->get_addressHashCode()].part.updateNum++;
	} else if (sqlInfo.part.type == stats::sql_op_delete) {
		record()->clientQueryMap[conn.clins()->get_addressHashCode()].part.deleteNum++;
	}

	//in trans.
	if (conn.record.type >= TRANS_QUERY_TYPE && conn.record.type < TRANS_QUERY_SUM) {
		conn.record.sqlSet.insert(conn.record.sqlInfo.sqlHashCode);
		record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode].part.trans++;
	}
}

int ProtocolBase::stat_parseAndStatSql(Connection& conn, std::string sqlText)
{
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
	conn.record.sqlInfo.sqlHashCode = this->find_preparedSqlHashCode(conn, preparedHandle);
	uif (conn.record.sqlInfo.sqlHashCode == (unsigned int)-1) {
		logs(Logger::ERR, "preparedHandle(%d) no sql", preparedHandle);
		return -1;
	}
	Record::SqlInfo& sqlInfo = record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode];
	conn.record.sqlInfo.sqlText = sqlInfo.sqlText;
	logs_logsql("prepare sql: %s, preparedHandle: %u", sqlInfo.sqlText.c_str(), preparedHandle);

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
	time_t ttime = SystemApi::system_millisecond();
	record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode].part.execTime += ttime - conn.record.startQueryTime;
	if ((conn.record.totalRow != rows) && (rows != 0)) {
		record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode].part.totalRow += rows;
	} else {
		record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode].part.totalRow += conn.record.totalRow;
	}
	conn.record.totalRow = 0;
}

//sql语句执行错误,这个函数应该与stat_recvFinishedRow互斥
void ProtocolBase::stat_executeErr(Connection& conn)
{
	time_t ttime = SystemApi::system_millisecond();
	record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode].part.execTime += ttime - conn.record.startQueryTime;

	record()->sqlInfoMap[conn.record.sqlInfo.sqlHashCode].part.fail++;
	record()->clientQueryMap[conn.clins()->get_addressHashCode()].part.queryFailNum++;
	record()->clientQueryMap[conn.clins()->get_addressHashCode()].add_failSqlList(conn.record.sqlInfo.sqlHashCode);
}
