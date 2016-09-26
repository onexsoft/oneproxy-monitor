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

#define record() Record::get_recordInstance()

class Record{
public:
	declare_type_alias(SqlOp, stats::SqlOp)
	declare_type_alias(SqlInfo, stats::SqlInfo)
	declare_type_alias(TableInfo, stats::TableInfo)
	declare_type_alias(TransInfo, stats::TransInfo)
	declare_type_alias(Thread_Info, stats::Thread_Info)
	declare_type_alias(SqlInfoPart, stats::SqlInfoPart)
	declare_type_alias(Record_Mutex, stats::Record_Mutex)
	declare_type_alias(TableMapInfo, stats::TableMapInfo)
	declare_type_alias(TransInfoPart, stats::TransInfoPart)
	declare_type_alias(TableInfoPart, stats::TableInfoPart)
	declare_type_alias(ClientQueryInfo, stats::ClientQueryInfo)
	declare_type_alias(HttpServerRecord, stats::HttpServerRecord)
	declare_type_alias(TableMapInfoPart, stats::TableMapInfoPart)
	declare_type_alias(ClientQueryInfoPart, stats::ClientQueryInfoPart)

	declare_type_alias(RecordMutexMap, std::map<std::string, Record_Mutex>)
	declare_type_alias(ThreadInfoMap, std::map<u_uint64, Thread_Info>)//key: thread_id
	declare_type_alias(RequestPageCountMap, std::map<std::string, u_uint64>)//key: page url, value:num
	declare_type_alias(ClientInfoMap, std::map<unsigned int, ClientQueryInfo>) //key: ip hashcode
	declare_type_alias(SqlInfoMap, std::map<unsigned int, SqlInfo>) ////key: hashcode
	declare_type_alias(TableInfoMap, std::map<std::string, TableInfo>)
	declare_type_alias(TableMapInfoMap, std::map<std::string, TableMapInfo>)
	declare_type_alias(TransInfoMap, std::map<unsigned int, TransInfo>)//key is sqlhashcode list hashcode

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

private:
	Record() {
		this->sum_clientConn = 0;
		this->sum_currentClientConn = 0;
		this->sum_handingClientConn = 0;
		this->sum_waitClientConn = 0;
		this->sum_closeClientConn = 0;
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

	SqlInfoMap sqlInfoMap;//sql语句查询的统计情况
	TransInfoMap transInfoMap; //事务的统计情况
	ThreadInfoMap threadInfoMap;//线程信息的统计
	ClientInfoMap clientQueryMap;//客户端查询信息的统计情况
	RecordMutexMap recordMutexMap; //锁使用情况的统计
	HttpServerRecord httpServerRecord;//http server的统计
	RequestPageCountMap httpRequestPageCount;//http server 请求页的统计情况

	static MutexLock mutex;
	int record_print_time_interval;//second
	static int realRecordTime;

	static Record *bakRecord;
	u_uint64 bakRecordStartTime;
};

#endif /* RECORD_H_ */
