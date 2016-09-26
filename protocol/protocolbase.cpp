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
	StringBuf& packet = conn.clins->get_recvData();
	int oldOffset = packet.get_offset();

	do{
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
		StringBuf& desPacket = *(conn.clins->get_bufPointer());
		for(; desPacket.get_offset() < desPacket.length(); ) {//循环处理，直到所有的数据处理完毕
			int type = this->get_packetType(desPacket);
			ProtocolBase::BaseHandleFuncMap::iterator it = this->frontHandleFunc.find(type);
			lif(it != this->frontHandleFunc.end()) {
				ProtocolHandleRetVal hret = (this->*it->second)(conn, desPacket);
				if(hret == HANDLE_RETURN_ERROR) {
					logs(Logger::INFO, "handle front packet error or need send direct, packet type is %d", (int)type);
					break;
				} else if (hret == HANDLE_RETURN_SEND_DIRECT) {
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

	packet.set_offset(oldOffset);
}

void ProtocolBase::protocol_backend(Connection& conn)
{
	StringBuf& packet = conn.servns->get_recvData();
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
		StringBuf& desPacket = *(conn.servns->get_bufPointer());
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
	uif(conn.clins == NULL)
		return -1;

	//默认情况下，直接设置需要处理的数据为接收到的数据包
	conn.clins->set_bufPointer(&(conn.clins->get_recvData()));
	return 0;
}

int ProtocolBase::protocol_initBackendStatPacket(Connection& conn)
{
	uif (conn.servns == NULL)
		return -1;

	conn.servns->set_bufPointer(&(conn.servns->get_recvData()));
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
void ProtocolBase::save_preparedSqlHashCode(unsigned int preparedHandle, unsigned int sqlHashCode)
{
	logs(Logger::INFO, "preparedHandle: %u, sqlHashCode: %u", preparedHandle, sqlHashCode);
	this->preparedHandleMap[preparedHandle] = sqlHashCode;
}

unsigned int ProtocolBase::find_preparedSqlHashCode(unsigned int preparedHandle)
{
	std::map<unsigned int, unsigned int>::iterator it = this->preparedHandleMap.find(preparedHandle);
	if (it == this->preparedHandleMap.end())
		return (unsigned int)-1;
	else
		return it->second;
}

void ProtocolBase::remove_preparedSqlHashCode(unsigned int preparedHandle)
{
	this->preparedHandleMap.erase(preparedHandle);
}

//返回sqlParser对象
SqlParser& ProtocolBase::get_sqlParser()
{
	return this->sqlParser;
}

unsigned int ProtocolBase::get_currentSqlHashCode(Connection& conn)
{
	return conn.record.sqlHashCode;
}

std::string& ProtocolBase::get_currentSqlText(Connection& conn)
{
	return conn.record.sqlText;
}

//此函数只是简单的调用解析器的parse函数，其他的事情不做。用户可以通过获取到sqlparser对象来获取其他的信息
int ProtocolBase::parse_sql(std::string sqlText)
{
	if(this->sqlParser.parse(sqlText.c_str()) == false){
		logs(Logger::ERR, "sql parser parse sql error");
		return -1;
	}
	return 0;
}

//此函数中调用parse函数后，并且通过解析器修改sql语句，通过modifySqlText返回替换sql语句的中的值为?后的sql语句。
int ProtocolBase::parse_sql(std::string sqlText, std::string& modifySqlText)
{
	if(this->sqlParser.parse(sqlText.c_str()) == false){
		logs(Logger::ERR, "sql parser parse sql error");
		return -1;
	}

	this->sqlParser.toPatternQuery(modifySqlText);
	return 0;
}

//统计读取到前端的数据
void ProtocolBase::stat_readFrontData(Connection& conn)
{
	record()->clientQueryMap[conn.clins->get_addressHashCode()].part.upDataSize += conn.clins->get_recvData().get_length();
}

//统计从后端返回的数据
void ProtocolBase::stat_readBackendData(Connection& conn)
{
	record()->clientQueryMap[conn.clins->get_addressHashCode()].part.downDataSize += conn.servns->get_recvData().get_length();
	record()->sqlInfoMap[conn.record.sqlHashCode].part.recvSize += conn.servns->get_recvData().get_length();
}

//开始事务
void ProtocolBase::stat_startTrans(Connection& conn)
{
	//record translation start data
	logs(Logger::INFO, "stat start trans");
	conn.record.trans_start_time = conn.record.startQueryTime;
//	conn.record.transStartTimeStr = conn.record.startQueryTimeStr;
	conn.record.sqlSet.insert(conn.record.sqlHashCode);
	conn.record.type = TRANS_QUERY_TYPE;
}

//结束事务
void ProtocolBase::stat_endTrans(Connection& conn, bool isRollBack)
{
	logs(Logger::INFO, "stat end trans");
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
		return;

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
	conn.record.type = SIMPLE_QUERY_TYPE;
}

void ProtocolBase::stat_saveSql(Connection& conn, std::string& sqlText)
{
	conn.record.sqlText = sqlText;
	conn.record.sqlHashCode = Tool::quick_hash_code(conn.record.sqlText.c_str(), conn.record.sqlText.length());

	if (this->sqlParser.isStartTrans()) {
		logs(Logger::INFO, "start trans");
		conn.record.transFlag = TRANS_BEGIN_TYPE;
	} else if (this->sqlParser.isCommit()) {
		conn.record.transFlag = TRANS_COMMIT_TYPE;
		logs(Logger::INFO, "COMMIT TRANS");
	} else if (this->sqlParser.isRollBack()) {
		conn.record.transFlag = TRANS_ROLLBACK_TYPE;
		logs(Logger::INFO, "ROLLBACK trans");
	} else {
		conn.record.transFlag = TRANS_NOTIN_TYPE;
		logs(Logger::INFO, "NOTIN trans");
	}

	Record::SqlInfo& sqlInfo = record()->sqlInfoMap[conn.record.sqlHashCode];
	if (sqlInfo.part.hashCode == 0) {
		sqlInfo.part.hashCode = conn.record.sqlHashCode;
		sqlInfo.sqlText = conn.record.sqlText;
		sqlInfo.part.tabs = this->sqlParser.tableCount();
		sqlInfo.part.type =  (Record::SqlOp)this->sqlParser.queryType();
		record()->clientQueryMap[conn.clins->get_addressHashCode()].add_sqlList(sqlInfo.part.hashCode);
		sqlInfo.add_clientSet(conn.clins->get_addressHashCode());

		//get table info
		for (unsigned int i = 0; i < sqlInfo.part.tabs; ++i) {
			const char* tmpTable = this->sqlParser.table(i)->table;
			sqlInfo.add_tableSet(std::string(tmpTable, strlen(tmpTable)));
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
	conn.record.sqlText.clear();
	logs_logsql("%s", sqlText.c_str());
	if (sqlText.length() <= 0) {
		logs(Logger::INFO, "sqltext is empty");
		return 0;
	}

	//1. 解析sql语句
	std::string tmpstr;
	if (this->parse_sql(sqlText, tmpstr)) {
		return -1;
	}

	//2. record
	this->stat_saveSql(conn, tmpstr);
	return 0;
}

//根据connection中的hashcode来统计执行的sql语句
void ProtocolBase::stat_executeSql(Connection& conn)
{
	//record time
	conn.record.startQueryTime = SystemApi::system_millisecond();
//	conn.record.startQueryTimeStr = SystemApi::system_timeStr();
	conn.record.totalRow = 0;

	if (conn.record.sqlHashCode <= 0)
		return;

	Record::SqlInfo& sqlInfo = record()->sqlInfoMap[conn.record.sqlHashCode];
	sqlInfo.part.exec++;

	//record client
	record()->clientQueryMap[conn.clins->get_addressHashCode()].part.queryNum++;
	if (sqlInfo.part.type == stats::sql_op_select) {
		record()->clientQueryMap[conn.clins->get_addressHashCode()].part.selectNum++;
	} else if (sqlInfo.part.type == stats::sql_op_insert) {
		record()->clientQueryMap[conn.clins->get_addressHashCode()].part.insertNum++;
	} else if (sqlInfo.part.type == stats::sql_op_update) {
		record()->clientQueryMap[conn.clins->get_addressHashCode()].part.updateNum++;
	} else if (sqlInfo.part.type == stats::sql_op_delete) {
		record()->clientQueryMap[conn.clins->get_addressHashCode()].part.deleteNum++;
	}

	//in trans.
	if (conn.record.type == TRANS_QUERY_TYPE) {
		conn.record.sqlSet.insert(conn.record.sqlHashCode);
		record()->sqlInfoMap[conn.record.sqlHashCode].part.trans++;
	} else if (conn.record.type == QUERY_TYPE_INIT) {
		conn.record.type = SIMPLE_QUERY_TYPE;
	}
}

int ProtocolBase::stat_parseAndStatSql(Connection& conn, std::string sqlText)
{
	conn.record.sqlText.clear();
	logs_logsql("%s", sqlText.c_str());
	if (sqlText.length() <= 0) {
		logs(Logger::INFO, "sqltext is empty");
		return 0;
	}

	//1. 解析sql语句
	std::string tmpstr;
	if (this->parse_sql(sqlText, tmpstr)) {
		return -1;
	}

	//2. 统计sql语句
	this->stat_executeSql(conn, tmpstr);

	return 0;
}

int ProtocolBase::stat_preparedSql(Connection& conn, unsigned int preparedHandle)
{
	//find sqlHashCode;
	conn.record.sqlHashCode = this->find_preparedSqlHashCode(preparedHandle);
	uif (conn.record.sqlHashCode == (unsigned int)-1) {
		logs(Logger::ERR, "preparedHandle(%d) no sql", preparedHandle);
		return -1;
	}
	Record::SqlInfo& sqlInfo = record()->sqlInfoMap[conn.record.sqlHashCode];
	conn.record.sqlText = sqlInfo.sqlText;
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
	record()->sqlInfoMap[conn.record.sqlHashCode].part.execTime += ttime - conn.record.startQueryTime;
	if ((conn.record.totalRow != rows) && (rows != 0)) {
		record()->sqlInfoMap[conn.record.sqlHashCode].part.totalRow += rows;
	} else {
		record()->sqlInfoMap[conn.record.sqlHashCode].part.totalRow += conn.record.totalRow;
	}
	conn.record.totalRow = 0;
}

//sql语句执行错误,这个函数应该与stat_recvFinishedRow互斥
void ProtocolBase::stat_executeErr(Connection& conn)
{
	time_t ttime = SystemApi::system_millisecond();
	record()->sqlInfoMap[conn.record.sqlHashCode].part.execTime += ttime - conn.record.startQueryTime;

	record()->sqlInfoMap[conn.record.sqlHashCode].part.fail++;
	record()->clientQueryMap[conn.clins->get_addressHashCode()].part.queryFailNum++;
	record()->clientQueryMap[conn.clins->get_addressHashCode()].add_failSqlList(conn.record.sqlHashCode);
}
