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

MutexLock Record::mutex;
int Record::realRecordTime = 10;
Record* Record::bakRecord = NULL;

Record* Record::get_recordInstance()
{
	static Record* record = NULL;

	if (record == NULL) {
		Record::mutex.set_name("record_lock");
		Record::mutex.lock();

		if (record == NULL) {
			record = new Record();
		}

		Record::mutex.unlock();
	}
	return record;
}

Record* Record::get_realTimeRecord()
{
	if (Record::bakRecord == NULL) {
		return Record::get_recordInstance();
	}
	return Record::get_diffRecord(Record::get_recordInstance(), Record::bakRecord);
}

void Record::bak_record()
{
	this->bakRecordStartTime = SystemApi::system_millisecond() / 1000;
	if (this->bakRecord == NULL) {
		this->bakRecord = new Record();
	}

	this->bakRecord->sum_clientConn = record()->sum_clientConn;
	this->bakRecord->sum_closeClientConn = record()->sum_closeClientConn;
	this->bakRecord->sum_currentClientConn = record()->sum_currentClientConn;
	this->bakRecord->sum_handingClientConn = record()->sum_handingClientConn;
	this->bakRecord->sum_waitClientConn = record()->sum_waitClientConn;

	this->bakRecord->sqlInfoMap = record()->sqlInfoMap;
	this->bakRecord->transInfoMap = record()->transInfoMap;
	this->bakRecord->threadInfoMap = record()->threadInfoMap;
	this->bakRecord->clientQueryMap = record()->clientQueryMap;
	this->bakRecord->recordMutexMap = record()->recordMutexMap;
	this->bakRecord->httpServerRecord = record()->httpServerRecord;
	this->bakRecord->httpRequestPageCount = record()->httpRequestPageCount;
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
		Record::SqlInfoMap::iterator it = newRecord->sqlInfoMap.begin();
		for (; it != newRecord->sqlInfoMap.end(); ++it) {
			Record::SqlInfoMap::iterator oit = oldRecord->sqlInfoMap.find(it->first);
			if (oit != oldRecord->sqlInfoMap.end()) {
				record.sqlInfoMap[it->first] = it->second - oit->second;
			} else {
				record.sqlInfoMap[it->first] = it->second;
			}
		}
	}

	{
		Record::TransInfoMap::iterator it = newRecord->transInfoMap.begin();
		for (; it != newRecord->transInfoMap.end(); ++it) {
			Record::TransInfoMap::iterator oit = oldRecord->transInfoMap.find(it->first);
			if (oit != oldRecord->transInfoMap.end()) {
				record.transInfoMap[it->first] = it->second - oit->second;
			} else {
				record.transInfoMap[it->first] = it->second;
			}
		}
	}

	{
		Record::ThreadInfoMap::iterator it = newRecord->threadInfoMap.begin();
		for (; it != newRecord->threadInfoMap.end(); ++it) {
			Record::ThreadInfoMap::iterator oit = oldRecord->threadInfoMap.find(it->first);
			if (oit != oldRecord->threadInfoMap.end()) {
				record.threadInfoMap[it->first] = it->second - oit->second;
			} else {
				record.threadInfoMap[it->first] = it->second;
			}
		}
	}

	{
		Record::ClientInfoMap::iterator it = newRecord->clientQueryMap.begin();
		for (; it != newRecord->clientQueryMap.end(); ++it) {
			Record::ClientInfoMap::iterator oit = oldRecord->clientQueryMap.find(it->first);
			if (oit != oldRecord->clientQueryMap.end()) {
				record.clientQueryMap[it->first] = it->second - oit->second;
			} else {
				record.clientQueryMap[it->first] = it->second;
			}
		}
	}

	{
		Record::RecordMutexMap::iterator it = newRecord->recordMutexMap.begin();
		for (; it != newRecord->recordMutexMap.end(); ++it) {
			Record::RecordMutexMap::iterator oit = oldRecord->recordMutexMap.find(it->first);
			if (oit != oldRecord->recordMutexMap.end()) {
				record.recordMutexMap[it->first] = it->second - oit->second;
			} else {
				record.recordMutexMap[it->first] = it->second;
			}
		}
	}

	{
		Record::RequestPageCountMap::iterator it = newRecord->httpRequestPageCount.begin();
		for (; it != newRecord->httpRequestPageCount.end(); ++it) {
			Record::RequestPageCountMap::iterator oit = oldRecord->httpRequestPageCount.find(it->first);
			if (oit != oldRecord->httpRequestPageCount.end()) {
				record.httpRequestPageCount[it->first] = it->second - oit->second;
			} else {
				record.httpRequestPageCount[it->first] = it->second;
			}
		}
	}

	return &record;
}

void Record::record_lock(MutexLock* mutexLock)
{
	Record::mutex.lock();
	if (mutexLock->get_name().size() > 0)
		this->recordMutexMap[mutexLock->get_name()].lockNum ++;
	Record::mutex.unlock();
}

void  Record::record_unlock(MutexLock* mutexLock)
{
	Record::mutex.lock();
	if (mutexLock->get_name().size() > 0)
		this->recordMutexMap[mutexLock->get_name()].unlockNum ++;
	Record::mutex.unlock();
}

void Record::record_acceptClientConn()
{
	this->sum_clientConn ++;
	this->sum_currentClientConn ++;
	this->sum_waitClientConn ++;
}

void Record::record_startHandingConn()
{
	this->sum_handingClientConn ++;
	this->sum_waitClientConn --;
}

void Record::record_closeClientConn()
{
	this->sum_closeClientConn ++;
	this->sum_handingClientConn --;
	this->sum_currentClientConn --;
}

void Record::record_threadStartHandingConn(u_uint64 threadId)
{
	Record::mutex.lock();
	this->threadInfoMap[threadId].sum_allClientConn++;
	this->threadInfoMap[threadId].sum_handingClientConn++;
	this->threadInfoMap[threadId].thread_id = threadId;
	Record::mutex.unlock();
}

void Record::record_threadFinishedConn(u_uint64 threadId)
{
	Record::mutex.lock();
	this->threadInfoMap[threadId].sum_handingClientConn--;
	this->threadInfoMap[threadId].sum_finishedClientConn++;
	Record::mutex.unlock();
}

void Record::record_threadFailConn(u_uint64 threadId)
{
	Record::mutex.lock();
	this->threadInfoMap[threadId].sum_failClientConn++;
	Record::mutex.unlock();
}

void Record::record_print()
{
	static int first = true;
	static time_t old_time, curr_time;

	if (first) {
		old_time = SystemApi::system_millisecond()/1000;
		first = false;
	}

	curr_time = SystemApi::system_millisecond()/1000;
	if (curr_time - old_time < this->record_print_time_interval)
		return;
	else
		old_time = curr_time;

	logs(Logger::INFO, "clientConn\t currentClientConn\t handingClientConn\t waitClientConn\t closeClientConn");
	logs(Logger::INFO, "%10lld, %17lld, %17lld, %14lld, %15lld", this->sum_clientConn, this->sum_currentClientConn,
			this->sum_handingClientConn, this->sum_waitClientConn, this->sum_closeClientConn);

	logs(Logger::INFO, "lockName\t lockNum\t unlockNum");
	RecordMutexMap::iterator it = recordMutexMap.begin();
	for (; it != recordMutexMap.end(); ++it) {
		logs(Logger::INFO, "%s\t %7lld\t %9lld", it->first.c_str(), it->second.lockNum, it->second.unlockNum);
	}

	logs(Logger::INFO, "threadid\t allClientConn\t handingClientConn\t failClientConn");
	ThreadInfoMap::iterator tit = threadInfoMap.begin();
	for (; tit != threadInfoMap.end(); ++tit) {
		logs(Logger::INFO, "%lld\t %lld\t %lld\t %lld", tit->second.thread_id,
				tit->second.sum_allClientConn, tit->second.sum_handingClientConn, tit->second.sum_failClientConn);
	}
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
{
	Record::mutex.lock();
	this->httpRequestPageCount[pageUrl] = this->httpRequestPageCount[pageUrl] + 1;
	Record::mutex.unlock();
}

void Record::record_clear()
{
	this->mutex.lock();
	delete this->bakRecord;
	this->bakRecord = NULL;

	this->bakRecordStartTime = 0;

	this->sum_clientConn = 0;
	this->sum_closeClientConn = 0;
	this->sum_currentClientConn = 0;
	this->sum_handingClientConn = 0;
	this->sum_waitClientConn = 0;
	RecordMutexMap::iterator it = recordMutexMap.begin();
	for (; it != recordMutexMap.end(); ++it) {
		it->second.clear();
	}
	ThreadInfoMap::iterator tit = threadInfoMap.begin();
	for (; tit != threadInfoMap.end(); ++tit) {
		tit->second.clear();
	}

	httpServerRecord.clear();

	Record::RequestPageCountMap::iterator hrpcit = httpRequestPageCount.begin();
	for (; hrpcit != httpRequestPageCount.end(); ++hrpcit) {
		hrpcit->second = 0;
	}

	ClientInfoMap::iterator cimit = this->clientQueryMap.begin();
	for (; cimit != this->clientQueryMap.end(); ++cimit) {
		cimit->second.clear();
	}

	SqlInfoMap::iterator simit = this->sqlInfoMap.begin();
	for (; simit != this->sqlInfoMap.end(); ++simit) {
		simit->second.clear();
	}
	this->mutex.unlock();
}

void Record::ClientQueryInfo::add_sqlList(unsigned int sqlHashCode)
{
//	logs(Logger::ERR, "ENTER XXXXXXXXXXXXXXXXXXX add_sqlList");
//	sqlListLock.lock();
	sqlList.insert(sqlHashCode);
//	sqlListLock.unlock();
//	logs(Logger::ERR, "FINISHED XXXXXXXXXXXXXXXXXXX add_sqlList");
}

void Record::ClientQueryInfo::add_failSqlList(unsigned int sqlHashCode)
{
//	logs(Logger::ERR, "ENTER XXXXXXXXXXXXXXXXXXX add_FAILsqlList");
//	failSqlListLock.lock();
	failSqlList.insert(sqlHashCode);
//	failSqlListLock.unlock();
//	logs(Logger::ERR, "FINISHED XXXXXXXXXXXXXXXXXXX add_failsqlList");
}

void Record::SqlInfo::add_clientSet(unsigned int clientHashCode)
{
//	logs(Logger::ERR, "ENTER XXXXXXXXXXXXXXXXXXX add_clientList");
//	this->clientSetLock.lock();
	this->clientSet.insert(clientHashCode);
//	this->clientSetLock.unlock();
//	logs(Logger::ERR, "finished XXXXXXXXXXXXXXXXXXX add_clientList");
}

void Record::SqlInfo::add_tableSet(std::string tableName)
{
//	logs(Logger::ERR, "ENTER XXXXXXXXXXXXXXXXXXX add_tableList");
//	this->tableSetLock.lock();
	this->tableSet.insert(tableName);
//	this->tableSetLock.unlock();
//	logs(Logger::ERR, "finished XXXXXXXXXXXXXXXXXXX add_tableList");
}


Record::SqlInfo* Record::find_sqlInfo(unsigned int sqlHashCode)
{
	Record::SqlInfoMap::iterator it = this->sqlInfoMap.find(sqlHashCode);
	if (it == this->sqlInfoMap.end())
		return NULL;
	else
		return &it->second;
}
