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
* @FileName: record.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月5日
*
*/

#include "record.h"
#include "systemapi.h"
#include "conf/config.h"
#include "db_define.h"
#include "tool.h"
#include <string>
#include <iostream>
#include <sstream>

namespace stats{
MutexLock Record::recordMutex;
int Record::realRecordTime = 10;
Record* Record::bakRecord = NULL;
MutexLock Record::bakRecordMutex;

DBManager Record::dbManager;

Record* Record::get_recordInstance()
{
	static Record* record = NULL;

	if (record == NULL) {
		Record::recordMutex.set_name("record_lock");
		Record::recordMutex.lock();

		if (record == NULL) {
			record = new Record();
		}

		Record::recordMutex.unlock();
	}
	return record;
}

Record* Record::get_realTimeRecord()
{
	Record* record = NULL;
	if (Record::bakRecord == NULL) {
		record =  Record::get_recordInstance();
	} else {
		Record::bakRecordMutex.lock();
		record = Record::get_diffRecord(Record::get_recordInstance(), Record::bakRecord);
		Record::bakRecordMutex.unlock();
	}
	return record;
}

void Record::bak_record()
{
	this->bakRecordMutex.lock();
	this->bakRecordStartTime = config()->get_globalSecondTime();
	if (this->bakRecord == NULL) {
		this->bakRecord = new Record();
	}

	this->bakRecord->sum_clientConn = record()->sum_clientConn;
	this->bakRecord->sum_closeClientConn = record()->sum_closeClientConn;
	this->bakRecord->sum_currentClientConn = record()->sum_currentClientConn;
	this->bakRecord->sum_handingClientConn = record()->sum_handingClientConn;
	this->bakRecord->sum_waitClientConn = record()->sum_waitClientConn;

	this->sqlInfoMapLock.lock();
	this->bakRecord->sqlInfoMapLock.lock();
	this->bakRecord->sqlInfoMap = record()->sqlInfoMap;
	this->bakRecord->sqlInfoMapLock.unlock();
	this->sqlInfoMapLock.unlock();

	this->transInfoMapLock.lock();
	this->bakRecord->transInfoMap = record()->transInfoMap;
	this->transInfoMapLock.unlock();

	this->threadInfoMapMutex.lock();
	this->bakRecord->threadInfoMap = record()->threadInfoMap;
	this->threadInfoMapMutex.unlock();

	this->clientQueryMapLock.lock();
	this->bakRecord->clientQueryMap = record()->clientQueryMap;
	this->clientQueryMapLock.unlock();

	this->recordMutexMapMutex.lock();
	this->bakRecord->recordMutexMap = record()->recordMutexMap;
	this->recordMutexMapMutex.unlock();

	this->httpServerMutex.lock();
	this->bakRecord->httpServerRecord = record()->httpServerRecord;
	this->bakRecord->httpRequestPageCount = record()->httpRequestPageCount;
	this->httpServerMutex.unlock();

	this->clientUserAppMapMutex.lock();
	this->bakRecord->clientUserAppMap = record()->clientUserAppMap;
	this->clientUserAppMapMutex.unlock();

	this->bakRecordMutex.unlock();
}

Record* Record::get_diffRecord(Record* newRecord, Record* oldRecord)
{
	static Record record;

	record.sum_clientConn = newRecord->sum_clientConn - oldRecord->sum_clientConn;
	record.sum_closeClientConn = newRecord->sum_closeClientConn - oldRecord->sum_closeClientConn;
	record.sum_currentClientConn = newRecord->sum_currentClientConn - oldRecord->sum_currentClientConn;
	record.sum_handingClientConn = newRecord->sum_handingClientConn - oldRecord->sum_handingClientConn;
	record.sum_waitClientConn = newRecord->sum_waitClientConn - oldRecord->sum_waitClientConn;
	record.httpServerRecord = newRecord->httpServerRecord - oldRecord->httpServerRecord;

	{
		newRecord->sqlInfoMapLock.lock();
		oldRecord->sqlInfoMapLock.lock();
		SqlInfoMap::iterator it = newRecord->sqlInfoMap.begin();
		for (; it != newRecord->sqlInfoMap.end(); ++it) {
			SqlInfoMap::iterator oit = oldRecord->sqlInfoMap.find(it->first);
			if (oit != oldRecord->sqlInfoMap.end()) {
				record.sqlInfoMap[it->first] = it->second - oit->second;
			} else {
				record.sqlInfoMap[it->first] = it->second;
			}
		}
		oldRecord->sqlInfoMapLock.unlock();
		newRecord->sqlInfoMapLock.unlock();
	}

	{
		newRecord->transInfoMapLock.lock();
		oldRecord->transInfoMapLock.lock();
		TransInfoMap::iterator it = newRecord->transInfoMap.begin();
		for (; it != newRecord->transInfoMap.end(); ++it) {
			TransInfoMap::iterator oit = oldRecord->transInfoMap.find(it->first);
			if (oit != oldRecord->transInfoMap.end()) {
				record.transInfoMap[it->first] = it->second - oit->second;
			} else {
				record.transInfoMap[it->first] = it->second;
			}
		}
		oldRecord->transInfoMapLock.unlock();
		newRecord->transInfoMapLock.unlock();
	}

	{
		newRecord->threadInfoMapMutex.lock();
		oldRecord->threadInfoMapMutex.lock();
		ThreadInfoMap::iterator it = newRecord->threadInfoMap.begin();
		for (; it != newRecord->threadInfoMap.end(); ++it) {
			ThreadInfoMap::iterator oit = oldRecord->threadInfoMap.find(it->first);
			if (oit != oldRecord->threadInfoMap.end()) {
				record.threadInfoMap[it->first] = it->second - oit->second;
			} else {
				record.threadInfoMap[it->first] = it->second;
			}
		}
		oldRecord->threadInfoMapMutex.unlock();
		newRecord->threadInfoMapMutex.unlock();
	}

	{
		newRecord->clientQueryMapLock.lock();
		oldRecord->clientQueryMapLock.lock();
		ClientInfoMap::iterator it = newRecord->clientQueryMap.begin();
		for (; it != newRecord->clientQueryMap.end(); ++it) {
			ClientInfoMap::iterator oit = oldRecord->clientQueryMap.find(it->first);
			if (oit != oldRecord->clientQueryMap.end()) {
				record.clientQueryMap[it->first] = it->second - oit->second;
			} else {
				record.clientQueryMap[it->first] = it->second;
			}
		}
		oldRecord->clientQueryMapLock.unlock();
		newRecord->clientQueryMapLock.unlock();
	}

	{
		newRecord->clientUserAppMapMutex.lock();
		oldRecord->clientUserAppMapMutex.lock();
		ClientUserAppInfoMap::iterator it = newRecord->clientUserAppMap.begin();
		for (; it != newRecord->clientUserAppMap.end(); ++it) {
			ClientUserAppInfoMap::iterator oit = oldRecord->clientUserAppMap.find(it->first);
			if (oit != oldRecord->clientUserAppMap.end()) {
				record.clientUserAppMap[it->first] = it->second - oit->second;
			} else {
				record.clientUserAppMap[it->first] = it->second;
			}
		}
		oldRecord->clientUserAppMapMutex.unlock();
		newRecord->clientUserAppMapMutex.unlock();
	}

	{
		newRecord->recordMutexMapMutex.lock();
		oldRecord->recordMutexMapMutex.lock();
		RecordMutexMap::iterator it = newRecord->recordMutexMap.begin();
		for (; it != newRecord->recordMutexMap.end(); ++it) {
			RecordMutexMap::iterator oit = oldRecord->recordMutexMap.find(it->first);
			if (oit != oldRecord->recordMutexMap.end()) {
				record.recordMutexMap[it->first] = it->second - oit->second;
			} else {
				record.recordMutexMap[it->first] = it->second;
			}
		}
		oldRecord->recordMutexMapMutex.unlock();
		newRecord->recordMutexMapMutex.unlock();
	}

	{
		newRecord->httpServerMutex.lock();
		oldRecord->httpServerMutex.lock();
		RequestPageCountMap::iterator it = newRecord->httpRequestPageCount.begin();
		for (; it != newRecord->httpRequestPageCount.end(); ++it) {
			RequestPageCountMap::iterator oit = oldRecord->httpRequestPageCount.find(it->first);
			if (oit != oldRecord->httpRequestPageCount.end()) {
				record.httpRequestPageCount[it->first] = it->second - oit->second;
			} else {
				record.httpRequestPageCount[it->first] = it->second;
			}
		}
		oldRecord->httpServerMutex.unlock();
		newRecord->httpServerMutex.unlock();
	}

	return &record;
}

void Record::record_lock(MutexLock* mutexLock)
{
	this->recordMutexMapMutex.lock();
	if (mutexLock->get_name().size() > 0)
		this->recordMutexMap[mutexLock->get_name()].lockNum ++;
	this->recordMutexMapMutex.unlock();
}

void  Record::record_unlock(MutexLock* mutexLock)
{
	this->recordMutexMapMutex.lock();
	if (mutexLock->get_name().size() > 0)
		this->recordMutexMap[mutexLock->get_name()].unlockNum ++;
	this->recordMutexMapMutex.unlock();
}

void Record::record_acceptClientConn()
{
	clientConnLock.lock();
	this->sum_clientConn ++;
	this->sum_currentClientConn ++;
	this->sum_waitClientConn ++;
	clientConnLock.unlock();
}

void Record::record_startHandingConn()
{
	clientConnLock.lock();
	this->sum_handingClientConn ++;
	this->sum_waitClientConn --;
	clientConnLock.unlock();
}

void Record::record_closeClientConn()
{
	clientConnLock.lock();
	this->sum_closeClientConn ++;
	this->sum_handingClientConn --;
	this->sum_currentClientConn --;
	clientConnLock.unlock();
}

void Record::record_clientConn2GlobalQueue()
{
	clientConnLock.lock();
	this->sum_waitInGlobalQueue ++;
	clientConnLock.unlock();
}

void Record::record_outGlobalQueue()
{
	clientConnLock.lock();
	this->sum_waitInGlobalQueue --;
	clientConnLock.unlock();
}

void Record::record_threadRecvConn(u_uint64 threadId)
{
	this->threadInfoMapMutex.lock();
	Thread_Info& tinfo = this->threadInfoMap[threadId];
	tinfo.sum_waitClientConn++;
	tinfo.sum_allClientConn++;
	tinfo.thread_id = threadId;
	this->threadInfoMapMutex.unlock();
}

void Record::record_threadStartHandingConn(u_uint64 threadId)
{
	this->threadInfoMapMutex.lock();
	Thread_Info& tinfo = this->threadInfoMap[threadId];
	tinfo.sum_waitClientConn--;
	tinfo.sum_handingClientConn++;
	this->threadInfoMapMutex.unlock();
}

void Record::record_threadFinishedConn(u_uint64 threadId)
{
	this->threadInfoMapMutex.lock();
	Thread_Info& tinfo = this->threadInfoMap[threadId];
	tinfo.sum_handingClientConn--;
	tinfo.sum_finishedClientConn++;
	this->threadInfoMapMutex.unlock();
}

void Record::record_threadFailConn(u_uint64 threadId)
{
	this->threadInfoMapMutex.lock();
	this->threadInfoMap[threadId].sum_failClientConn++;
	this->threadInfoMapMutex.unlock();
}

void Record::record_print()
{
	static int first = true;
	static time_t old_time, curr_time;

	if (first) {
		old_time = config()->get_globalSecondTime();
		first = false;
	}

	curr_time = config()->get_globalSecondTime();
	if (curr_time - old_time < this->record_print_time_interval)
		return;
	else
		old_time = curr_time;

	logs(Logger::INFO, "clientConn\t currentClientConn\t handingClientConn\t waitClientConn\t closeClientConn");
	logs(Logger::INFO, "%10lld, %17lld, %17lld, %14lld, %15lld", this->sum_clientConn, this->sum_currentClientConn,
			this->sum_handingClientConn, this->sum_waitClientConn, this->sum_closeClientConn);

	logs(Logger::INFO, "lockName\t lockNum\t unlockNum");
	this->recordMutexMapMutex.lock();
	RecordMutexMap::iterator it = recordMutexMap.begin();
	for (; it != recordMutexMap.end(); ++it) {
		logs(Logger::INFO, "%s\t %7lld\t %9lld", it->first.c_str(), it->second.lockNum, it->second.unlockNum);
	}
	this->recordMutexMapMutex.unlock();

	logs(Logger::INFO, "threadid\t allClientConn\t handingClientConn\t failClientConn");
	this->threadInfoMapMutex.lock();
	ThreadInfoMap::iterator tit = threadInfoMap.begin();
	for (; tit != threadInfoMap.end(); ++tit) {
		logs(Logger::INFO, "%lld\t %lld\t %lld\t %lld", tit->second.thread_id,
				tit->second.sum_allClientConn, tit->second.sum_handingClientConn, tit->second.sum_failClientConn);
	}
	this->threadInfoMapMutex.unlock();
}

void Record::record_httpServerClientConnect()
{
	this->httpServerRecord.client_connect_num ++;
	this->httpServerRecord.client_current_connect_num ++;
}
void Record::record_httpServerRequestPage()
{
	this->httpServerRecord.client_request_page_num ++;
}
void Record::record_httpServerClientCloseConnect()
{
	this->httpServerRecord.client_current_connect_num --;
}

void Record::record_httpServerRequestPage(std::string pageUrl)
{//http server is single thread, not need lock.
	this->httpRequestPageCount[pageUrl] = this->httpRequestPageCount[pageUrl] + 1;
}

void Record::record_clear()
{
	this->bakRecordMutex.lock();
	delete this->bakRecord;
	this->bakRecord = NULL;
	this->bakRecordMutex.unlock();

	this->bakRecordStartTime = 0;

	this->sum_clientConn = 0;
	this->sum_closeClientConn = 0;
	this->sum_currentClientConn = 0;
	this->sum_handingClientConn = 0;
	this->sum_waitClientConn = 0;

	httpServerMutex.lock();
	httpServerRecord.clear();
	RequestPageCountMap::iterator hrpcit = httpRequestPageCount.begin();
	for (; hrpcit != httpRequestPageCount.end(); ++hrpcit) {
		hrpcit->second = 0;
	}
	httpServerMutex.unlock();

	this->recordMutexMapMutex.lock();
	RecordMutexMap::iterator it = recordMutexMap.begin();
	for (; it != recordMutexMap.end(); ++it) {
		it->second.clear();
	}
	this->recordMutexMapMutex.unlock();

	this->threadInfoMapMutex.lock();
	ThreadInfoMap::iterator tit = threadInfoMap.begin();
	for (; tit != threadInfoMap.end(); ++tit) {
		tit->second.clear();
	}
	this->threadInfoMapMutex.unlock();

	this->clientQueryMapLock.lock();
	ClientInfoMap::iterator cimit = this->clientQueryMap.begin();
	for (; cimit != this->clientQueryMap.end(); ++cimit) {
		cimit->second.clear();
	}
	this->clientQueryMapLock.unlock();

	this->sqlInfoMapLock.lock();
	SqlInfoMap::iterator simit = this->sqlInfoMap.begin();
	for (; simit != this->sqlInfoMap.end(); ++simit) {
		simit->second.clear();
	}
	this->sqlInfoMapLock.unlock();
}

SqlInfo* Record::find_sqlInfo(unsigned int sqlHashCode)
{
	SqlInfo* sqlInfo = NULL;

	this->sqlInfoMapLock.lock();
	SqlInfoMap::iterator it = this->sqlInfoMap.find(sqlHashCode);
	if (it == this->sqlInfoMap.end())
		sqlInfo = NULL;
	else
		sqlInfo =  &it->second;
	this->sqlInfoMapLock.unlock();

	return sqlInfo;
}

void Record::record_clientQueryRecvSize(Connection& conn, unsigned int hashCode,
		unsigned int size, ClientQueryInfo* clientInfo)
{
	if (clientInfo == NULL)
		clientInfo = this->record_getClientQueryInfo(hashCode);
	if (clientInfo != NULL) {
		clientInfo->part.downDataSize += size;
	}

	if (this->is_save2DB() && conn.connection_hashcode > 0) {
		ClientInfoT* cit = ClientInfoT::create_instance();
		cit->connHashCode = conn.connection_hashcode;
		cit->hashcode = hashCode;
		cit->recvSize = size;
		cit->sendSize = 0;

		DBDataT dt;
		dt.type = DB_DATA_TYPE_CLIENT_INFO_UPDATE;
		dt.data = cit;
		Record::dbManager.add_taskData(dt);
	}
}

void Record::record_clientQuerySendSize(Connection& conn, unsigned int hashCode,
		unsigned int size, ClientQueryInfo* clientInfo)
{
	if (clientInfo == NULL)
		clientInfo = this->record_getClientQueryInfo(hashCode);
	if (clientInfo != NULL) {
		clientInfo->part.upDataSize += size;
	}

	if (this->is_save2DB() && conn.connection_hashcode > 0) {
		ClientInfoT* cit = ClientInfoT::create_instance();
		cit->connHashCode = conn.connection_hashcode;
		cit->hashcode = hashCode;
		cit->recvSize = 0;
		cit->sendSize = size;

		DBDataT dt;
		dt.type = DB_DATA_TYPE_CLIENT_INFO_UPDATE;
		dt.data = cit;
		Record::dbManager.add_taskData(dt);
	}
}

void Record::record_clientQueryAddSql(Connection& conn, ClientQueryInfo* clientInfo) {
	if (clientInfo == NULL) {
		clientInfo = this->record_getClientQueryInfo(conn.clins()->get_addressHashCode());
	}
	if (clientInfo != NULL) {
		this->clientQueryMapLock.lock();
		clientInfo->sqlList.insert(conn.record.sqlInfo.sqlHashCode);
		this->clientQueryMapLock.unlock();
	}

	if (this->is_save2DB() && conn.connection_hashcode > 0 && conn.currExecSqlHash > 0) {
		ClientSqlRetS* csrs = ClientSqlRetS::create_instance();
		csrs->conn_hashcode = conn.connection_hashcode;
		csrs->sql_hashcode = conn.record.sqlInfo.sqlHashCode;
		csrs->sqlexec_hashcode = conn.currExecSqlHash;

		DBDataT dt;
		dt.type = DB_DATA_TYPE_CLIENT_ADD_SQL;
		dt.data = csrs;
		Record::dbManager.add_taskData(dt);
	}
}

void Record::record_clientQueryExec(unsigned int sqlHashCode,
		unsigned int clientAddressHashCode, ClientQueryInfo* clientInfo)
{
	SqlInfo* sqlInfo = this->find_sqlInfo(sqlHashCode);
	if (sqlInfo == NULL)
		return;

	if (clientInfo == NULL) {
		clientInfo = this->record_getClientQueryInfo(clientAddressHashCode);
	}
	if (clientInfo != NULL) {
		clientInfo->part.queryNum ++;
		if (sqlInfo->part.type == stats::sql_op_select) {
			clientInfo->part.selectNum++;
		} else if (sqlInfo->part.type == stats::sql_op_insert) {
			clientInfo->part.insertNum++;
		} else if (sqlInfo->part.type == stats::sql_op_update) {
			clientInfo->part.updateNum++;
		} else if (sqlInfo->part.type == stats::sql_op_delete) {
			clientInfo->part.deleteNum++;
		}
		return;
	}
}

void Record::record_clientQueryExecFail(Connection& conn, ClientQueryInfo* clientInfo)
{
	if (clientInfo == NULL)
		clientInfo = this->record_getClientQueryInfo(conn.clins()->get_addressHashCode());
	if (clientInfo != NULL) {
		clientInfo->part.queryFailNum++;
		this->clientQueryMapLock.lock();
		clientInfo->failSqlList.insert(conn.record.sqlInfo.sqlHashCode);
		this->clientQueryMapLock.unlock();
	}
}

void Record::record_clientQueryOnLineTime(unsigned int hashCode, u_uint64 time, ClientQueryInfo* clientInfo)
{
	if (clientInfo == NULL)
		clientInfo = this->record_getClientQueryInfo(hashCode);
	if (clientInfo)
		clientInfo->part.onLineTime += time;
}

void Record::record_clientQueryOffLine(unsigned int hashCode, ClientQueryInfo* clientInfo)
{
	if (clientInfo == NULL)
		clientInfo = this->record_getClientQueryInfo(hashCode);
	if (clientInfo) {
		clientInfo->part.onLineStatus = false;
		clientInfo->part.start_connect_time = 0;//断开连接时，设置为0.再排序时，排在最后
	}
}

void Record::record_clientQueryAllocServerFail(unsigned int hashCode, ClientQueryInfo* clientInfo)
{
	if (clientInfo == NULL)
		clientInfo = this->record_getClientQueryInfo(hashCode);
	if (clientInfo) {
		clientInfo->part.connServerFail ++;
	}
}

void Record::record_clientQueryAddNewClient(unsigned int connHashCode, unsigned int hashCode,
		std::string caddress, int cport, std::string saddress, int sport)
{
	this->clientQueryMapLock.lock();
	ClientQueryInfo* clientInfo = &this->clientQueryMap[hashCode];
	clientInfo->part.hashCode = hashCode;
	clientInfo->part.connectNum ++;
	clientInfo->ipAddr = caddress;
	clientInfo->part.onLineStatus = true;
	clientInfo->part.start_connect_time = config()->get_globalMillisecondTime();
	clientInfo->latest_connect_time = (u_uint64)SystemApi::system_time();
	this->clientQueryMapLock.unlock();

	if (!this->is_save2DB() || connHashCode <= 0)
		return;


	ClientInfoT* cit = ClientInfoT::create_instance();
	cit->connHashCode = connHashCode;
	cit->clientIp = caddress;
	cit->cport = cport;
	cit->hashcode = hashCode;
	cit->serverIp = saddress;
	cit->sport = sport;
	cit->start_connect_time = clientInfo->part.start_connect_time;
	cit->sendSize = 0;
	cit->recvSize = 0;

	DBDataT dt;
	dt.type = DB_DATA_TYPE_CLIENT_INFO;
	dt.data = cit;
	Record::dbManager.add_taskData(dt);

}

ClientQueryInfo* Record::record_getClientQueryInfo(unsigned int hashCode)
{
	ClientQueryInfo* info = NULL;
	this->clientQueryMapLock.lock();
	ClientInfoMap::iterator it = this->clientQueryMap.find(hashCode);
	if (it == this->clientQueryMap.end()) {
		this->clientQueryMapLock.unlock();
		return info;
	}
	info = &(it->second);
	this->clientQueryMapLock.unlock();
	return info;
}

void Record::record_sqlInfoRecvSize(Connection& conn, unsigned int hashCode,
		unsigned int size, SqlInfo* sqlInfo)
{
	if (sqlInfo == NULL) {
		sqlInfo = this->record_getSqlInfo(hashCode);
	}
	if (sqlInfo) {
		sqlInfo->part.recvSize += size;
	}

	if (this->is_save2DB() && conn.currExecSqlHash > 0) {
		SqlExecS *ses = SqlExecS::create_instance();
		ses->sqlExecHashCode = conn.currExecSqlHash;
		ses->connHashCode = conn.connection_hashcode;
		ses->recvSize = size;
		DBDataT dt;
		dt.type = DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_DATASIZE;
		dt.data = ses;
		Record::dbManager.add_taskData(dt);
	}
}

void Record::record_sqlInfoAddSql(Connection& conn)
{
	this->sqlInfoMapLock.lock();
	conn.sessData.sqlInfo = &this->sqlInfoMap[conn.record.sqlInfo.sqlHashCode];
	if (conn.sessData.sqlInfo->part.hashCode == 0) {
		conn.sessData.sqlInfo->part.hashCode = conn.record.sqlInfo.sqlHashCode;
		conn.sessData.sqlInfo->sqlText = conn.record.sqlInfo.sqlText;
		conn.sessData.sqlInfo->part.tabs = conn.record.sqlInfo.tableCount;
		conn.sessData.sqlInfo->part.type =  (SqlOp)conn.record.sqlInfo.queryType;
		conn.sessData.sqlInfo->clientSet.insert(conn.clins()->get_addressHashCode());
		//set table info
		for (unsigned int i = 0; i < conn.sessData.sqlInfo->part.tabs; ++i) {
			conn.sessData.sqlInfo->tableSet.insert(conn.record.sqlInfo.tableNameVec.at(i));
		}
	}
	this->sqlInfoMapLock.unlock();

	if (!this->is_save2DB())
		return;

	SqlInfoS* sis = SqlInfoS::create_instance();
	sis->execTime = config()->get_globalMillisecondTime();
	sis->connHashCode = conn.connection_hashcode;
	sis->sqlHashCode = conn.sessData.sqlInfo->part.hashCode;
	sis->queryType = (int)conn.sessData.sqlInfo->part.type;
	sis->sql = conn.sessData.sqlInfo->sqlText;
	sis->tabCnt = conn.sessData.sqlInfo->part.tabs;
	for (unsigned int i = 0; i < conn.sessData.sqlInfo->part.tabs; ++i) {
		sis->tabNameVec.push_back(conn.record.sqlInfo.tableNameVec.at(i));
	}
	{
		//generate execute sql hash code.
		std::stringstream ss;
		ss << sis->execTime << sis->connHashCode << sis->sqlHashCode;
		std::string str = ss.str();
		sis->sqlExecHashCode = Tool::quick_hash_code(str.c_str(), str.length());
		conn.currExecSqlHash = sis->sqlExecHashCode;
	}

	DBDataT dt;
	dt.type = DB_DATA_TYPE_CLIENT_EXEC_SQL;
	dt.data = sis;
	Record::dbManager.add_taskData(dt);
}

void Record::record_sqlInfoExec(unsigned int hashCode, SqlInfo* sqlInfo)
{
	if(sqlInfo == NULL) {
		sqlInfo = this->record_getSqlInfo(hashCode);
	}
	if(sqlInfo) {
		sqlInfo->part.exec++;
	}
}

void Record::record_sqlInfoExecTran(Connection& conn, unsigned int hashCode, SqlInfo* sqlInfo)
{
	if (sqlInfo == NULL) {
		sqlInfo = this->record_getSqlInfo(hashCode);
	}
	if (sqlInfo) {
		sqlInfo->part.trans++;
	}

	if (this->is_save2DB() && conn.currExecSqlHash > 0) {
		SqlExecS *ses = SqlExecS::create_instance();
		ses->sqlExecHashCode = conn.currExecSqlHash;
		ses->connHashCode = conn.connection_hashcode;
		ses->inTrans = 1;
		DBDataT dt;
		dt.type = DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_TRANS;
		dt.data = ses;
		Record::dbManager.add_taskData(dt);
	}
}

void Record::record_sqlInfoRecvResult(Connection& conn, unsigned int rows, SqlInfo* sqlInfo)
{
	time_t ttime = config()->get_globalMillisecondTime();
	int tmpRows = 0;
	if (sqlInfo == NULL)
		sqlInfo = this->record_getSqlInfo(conn.record.sqlInfo.sqlHashCode);
	if (sqlInfo) {
		sqlInfo->part.execTime += ttime - conn.record.startQueryTime;
		if ((conn.record.totalRow != rows) && (rows != 0)) {
			sqlInfo->part.totalRow += rows;
			tmpRows = rows;
		} else {
			tmpRows = conn.record.totalRow;
			sqlInfo->part.totalRow += conn.record.totalRow;
		}
	}

	if (this->is_save2DB() && conn.currExecSqlHash > 0) {
		SqlExecS *ses = SqlExecS::create_instance();
		ses->sqlExecHashCode = conn.currExecSqlHash;
		ses->connHashCode = conn.connection_hashcode;
		ses->rowNum = tmpRows;
		DBDataT dt;
		dt.type = DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_ROWNUM;
		dt.data = ses;
		Record::dbManager.add_taskData(dt);
	}
}

void Record::record_sqlInfoExecFail(Connection& conn, SqlInfo* sqlInfo)
{
	time_t ttime = config()->get_globalMillisecondTime();
	if (sqlInfo == NULL)
		sqlInfo = this->record_getSqlInfo(conn.record.sqlInfo.sqlHashCode);
	if (sqlInfo) {
		sqlInfo->part.execTime += ttime - conn.record.startQueryTime;
		sqlInfo->part.fail++;
	}

	if (this->is_save2DB() && conn.currExecSqlHash > 0) {
		SqlExecS *ses = SqlExecS::create_instance();
		ses->sqlExecHashCode = conn.currExecSqlHash;
		ses->connHashCode = conn.connection_hashcode;
		ses->fail = 1;
		DBDataT dt;
		dt.type = DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_FAIL;
		dt.data = ses;
		Record::dbManager.add_taskData(dt);
	}
}

SqlInfo* Record::record_getSqlInfo(unsigned int hashCode)
{
	SqlInfo* info = NULL;
	this->sqlInfoMapLock.lock();
	SqlInfoMap::iterator it = this->sqlInfoMap.find(hashCode);
	if (it == this->sqlInfoMap.end()) {
		this->sqlInfoMapLock.unlock();
		return info;
	}
	info = &(it->second);
	this->sqlInfoMapLock.unlock();
	return info;
}

void Record::record_transInfoEndTrans(unsigned int transHashCode, Connection& conn)
{
	TransInfoS* tis = TransInfoS::create_instance();

	u_uint64 finishedTime = config()->get_globalMillisecondTime();
	this->transInfoMapLock.lock();
	TransInfo& info = this->transInfoMap[transHashCode];
	{
		std::set<unsigned int>::iterator it = conn.record.sqlSet.begin();
		for(; it != conn.record.sqlSet.end(); ++it) {
			info.sqlHashCode.insert(*it);
			tis->sqlVec.push_back(*it);
		}
	}

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
	this->transInfoMapLock.unlock();
	tis->start_time = conn.record.trans_start_time;
	tis->end_time = config()->get_globalMillisecondTime();
	tis->connHashCode = conn.connection_hashcode;
	tis->transHashCode = transHashCode;

	if (this->is_save2DB() && tis->connHashCode > 0) {
		DBDataT dt;
		dt.type = DB_DATA_TYPE_CLIENT_EXEC_TRANS;
		dt.data = tis;
		Record::dbManager.add_taskData(dt);
	} else {
		tis->free_instance();
	}
}

void Record::record_clientUserAppInfoAdd(std::string hostName,
			std::string userName, std::string appName, unsigned int hashCode)
{
	this->clientUserAppMapMutex.lock();
	ClientUserAppInfo& tmpMap = this->clientUserAppMap[hashCode];
	tmpMap.appName = appName;
	tmpMap.hostName = hostName;
	tmpMap.userName = userName;
	tmpMap.part.loginTimes ++;
	tmpMap.hashCode = hashCode;
	this->clientUserAppMapMutex.unlock();

}

void Record::record_clientUserAppInfoAddSql(unsigned int hashCode, unsigned int sqlHashCode)
{
	this->clientUserAppMapMutex.lock();
	this->clientUserAppMap[hashCode].sqlList.insert(sqlHashCode);
	this->clientUserAppMapMutex.unlock();
}
}
