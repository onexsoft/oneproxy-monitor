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
* @FileName: record.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月5日
*
*/

#ifndef RECORD_H_
#define RECORD_H_

#include "logger.h"
#include "mutexlock.h"
#include "record_define.h"
#include "connection.h"
#include "uspinlock.h"
#include "dbmanager.h"

#define record() stats::Record::get_recordInstance()

namespace stats{
declare_type_alias(RecordMutexMap, std::map<std::string, Record_Mutex>)
declare_type_alias(ThreadInfoMap, std::map<u_uint64, Thread_Info>)//key: thread_id
declare_type_alias(RequestPageCountMap, std::map<std::string, u_uint64>)//key: page url, value:num
declare_type_alias(ClientInfoMap, std::map<unsigned int, ClientQueryInfo>) //key: ip hashcode
declare_type_alias(ClientUserAppInfoMap, std::map<unsigned int, ClientUserAppInfo>)
declare_type_alias(SqlInfoMap, std::map<unsigned int, SqlInfo>) ////key: hashcode
declare_type_alias(TableInfoMap, std::map<std::string, TableInfo>)
declare_type_alias(TableMapInfoMap, std::map<std::string, TableMapInfo>)
declare_type_alias(TransInfoMap, std::map<unsigned int, TransInfo>)//key is sqlhashcode list hashcode

class Record{
public:
	static Record* get_recordInstance();
	static Record* get_realTimeRecord();
	void bak_record();
	static Record* get_diffRecord(Record* newRecord, Record* oldRecord);

	void record_lock(MutexLock* mutexLock);
	void record_unlock(MutexLock* mutexLock);
	void record_acceptClientConn();
	void record_startHandingConn();
	void record_closeClientConn();
	void record_clientConn2GlobalQueue();
	void record_outGlobalQueue();
	void record_threadRecvConn(u_uint64 threadId);
	void record_threadStartHandingConn(u_uint64 threadId);
	void record_threadFinishedConn(u_uint64 threadId);
	void record_threadFailConn(u_uint64 threadId);
	void record_print();
	void record_httpServerClientConnect();
	void record_httpServerRequestPage();
	void record_httpServerClientCloseConnect();
	void record_httpServerRequestPage(std::string pageUrl);
	void record_clear();
	SqlInfo* find_sqlInfo(unsigned int sqlHashCode);
	void record_setPrintIntervalTime(int time) {
		this->record_print_time_interval = time;
	}

	void record_clientQueryRecvSize(Connection& conn, unsigned int hashCode,
			unsigned int size, ClientQueryInfo* clientInfo = NULL);
	void record_clientQuerySendSize(Connection& conn, unsigned int hashCode, unsigned int size,
			ClientQueryInfo* clientInfo = NULL);
	void record_clientQueryAddSql(Connection& conn, ClientQueryInfo* clientInfo = NULL);
	void record_clientQueryExec(unsigned int sqlHashCode, unsigned int clientAddressHashCode,
			ClientQueryInfo* clientInfo = NULL);
	void record_clientQueryExecFail(Connection& conn, ClientQueryInfo* clientInfo = NULL);
	void record_clientQueryOnLineTime(unsigned int hashCode, u_uint64 time, ClientQueryInfo* clientInfo = NULL);
	void record_clientQueryOffLine(unsigned int hashCode, ClientQueryInfo* clientInfo = NULL);
	void record_clientQueryAllocServerFail(unsigned int hashCode, ClientQueryInfo* clientInfo = NULL);
	void record_clientQueryAddNewClient(unsigned int connHashCode, unsigned int hashCode,
			std::string caddress, int cport, std::string saddress, int sport);
	ClientQueryInfo* record_getClientQueryInfo(unsigned int hashCode);

	void record_sqlInfoRecvSize(Connection& conn, unsigned int hashCode,
			unsigned int size, SqlInfo* sqlInfo = NULL);
	void record_sqlInfoAddSql(Connection& conn);
	void record_sqlInfoExec(unsigned int hashCode, SqlInfo* sqlInfo = NULL);
	void record_sqlInfoExecTran(Connection& conn, unsigned int hashCode, SqlInfo* sqlInfo = NULL);
	void record_sqlInfoRecvResult(Connection& conn, unsigned int rows, SqlInfo* sqlInfo = NULL);
	void record_sqlInfoExecFail(Connection& conn, SqlInfo* sqlInfo = NULL);
	SqlInfo* record_getSqlInfo(unsigned int hashCode);

	void record_transInfoEndTrans(unsigned int hashCode, Connection& conn);

	void record_clientUserAppInfoAdd(std::string hostName,
			std::string userName, std::string appName, unsigned int hashCode);
	void record_clientUserAppInfoAddSql(unsigned int hashCode, unsigned int sqlHashCode);

	void record_stop(){Record::dbManager.set_stop();}
private:
	Record() {
		this->sum_clientConn = 0;
		this->sum_currentClientConn = 0;
		this->sum_handingClientConn = 0;
		this->sum_waitClientConn = 0;
		this->sum_closeClientConn = 0;
		this->sum_waitInGlobalQueue = 0;
		this->record_print_time_interval = 5;
		this->bakRecord = NULL;
		this->bakRecordStartTime = 0;
	}
	~Record() {
	}

public:
	u_uint64 sum_clientConn;//从系统启动到目前总共客户端进行了多少次连接
	u_uint64 sum_currentClientConn;//当前连接到系统中的客户端的数量
	u_uint64 sum_handingClientConn;//正在处理的客户端连接
	u_uint64 sum_waitClientConn;//正在等待的客户端连接
	u_uint64 sum_closeClientConn;//从系统启动到目前关闭的客户端连接
	u_uint64 sum_waitInGlobalQueue;//统计全局队列中的任务数
	USpinLock clientConnLock;

	SqlInfoMap sqlInfoMap;//sql语句查询的统计情况
	USpinLock sqlInfoMapLock;

	TransInfoMap transInfoMap; //事务的统计情况
	USpinLock transInfoMapLock;

	ThreadInfoMap threadInfoMap;//线程信息的统计
	MutexLock threadInfoMapMutex;

	ClientInfoMap clientQueryMap;//客户端查询信息的统计情况
	USpinLock clientQueryMapLock;

	RecordMutexMap recordMutexMap; //锁使用情况的统计
	MutexLock recordMutexMapMutex;

	HttpServerRecord httpServerRecord;//http server的统计
	RequestPageCountMap httpRequestPageCount;//http server 请求页的统计情况
	MutexLock httpServerMutex;

	ClientUserAppInfoMap clientUserAppMap; //按照user ,host, app name进行统计
	MutexLock clientUserAppMapMutex;

	static MutexLock recordMutex;
	int record_print_time_interval;//second
	static int realRecordTime;

	static MutexLock bakRecordMutex;
	static Record *bakRecord;
	u_uint64 bakRecordStartTime;

	static DBManager dbManager;
};
}

#endif /* RECORD_H_ */
