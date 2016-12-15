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
* @FileName: record_define.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月20日
*
*/

#ifndef STATS_RECORD_DEFINE_H_
#define STATS_RECORD_DEFINE_H_

#include <map>
#include <string>
#include <list>
#include <iostream>
#include <set>
#include <string.h>

#include "define.h"

namespace stats{
//锁使用情况的统计
typedef struct record_mutex_t{
	u_uint64 lockNum;
	u_uint64 unlockNum;
	void clear() {
		this->lockNum = 0;
		this->unlockNum = 0;
	}
	bool is_show() {
		if (this->lockNum == 0 && this->unlockNum == 0)
			return false;
		return true;
	}
	record_mutex_t operator-(record_mutex_t& a) {
		record_mutex_t rmt;
		rmt.lockNum = this->lockNum - a.lockNum;
		rmt.unlockNum = this->unlockNum - a.unlockNum;
		return rmt;
	}
} Record_Mutex;

//线程信息统计
typedef struct thread_info_t{
	u_uint64 sum_allClientConn; //当前线程所处理的所有连接
	u_uint64 sum_handingClientConn;//正在处理的连接数
	u_uint64 sum_finishedClientConn;//完成的连接数
	u_uint64 sum_failClientConn;//处理 失败的连接数
	u_uint64 sum_waitClientConn;//正在等待的连接数
	u_uint64 thread_id;
	void clear() {
		this->sum_allClientConn = 0;
		this->sum_handingClientConn = 0;
		this->sum_finishedClientConn = 0;
		this->sum_failClientConn = 0;
		this->sum_waitClientConn = 0;
	}
	bool is_show() {
		return true;
	}
	thread_info_t operator-(thread_info_t& a) {
		thread_info_t tit;
		tit.sum_allClientConn = this->sum_allClientConn - a.sum_allClientConn;
		tit.sum_handingClientConn = this->sum_handingClientConn - a.sum_handingClientConn;
		tit.sum_finishedClientConn = this->sum_finishedClientConn - a.sum_finishedClientConn;
		tit.sum_failClientConn = this->sum_failClientConn - a.sum_failClientConn;
		tit.sum_waitClientConn = this->sum_waitClientConn - a.sum_waitClientConn;
		tit.thread_id = this->thread_id;
		return tit;
	}
}Thread_Info;

//http server 统计
typedef struct http_server_record_t{
	u_uint64 client_connect_num;//客户端连接次数
	u_uint64 client_request_page_num;//客户端请求页面次数
	u_uint64 client_current_connect_num;//当前连接的客户端数量
	http_server_record_t() {
		this->clear();
	}
	void clear() {
		this->client_connect_num = 0;
		this->client_request_page_num = 0;
		this->client_current_connect_num = 0;
	}
	struct http_server_record_t operator - (struct http_server_record_t& a) {
		struct http_server_record_t hsrt;
		hsrt.client_connect_num = this->client_connect_num - a.client_connect_num;
		hsrt.client_request_page_num = this->client_request_page_num - a.client_request_page_num;
		hsrt.client_current_connect_num = this->client_current_connect_num - a.client_current_connect_num;
		return hsrt;
	}
}HttpServerRecord;

typedef struct _client_query_info_part_t{
	unsigned int hashCode;
	u_uint64 connectNum; //连接次数
	u_uint64 connServerFail; //连接到服务端失败的次数
	u_uint64 queryNum; //查询的总次数
	u_uint64 queryFailNum; //查询失败的总次数
	u_uint64 trxNum;//完整事务查询的次数
	u_uint64 selectNum; //select的次数
	u_uint64 insertNum; //insert的次数
	u_uint64 updateNum; //update的次数
	u_uint64 deleteNum; //delete的次数
	u_uint64 onLineTime; //在线累计时间，单位毫秒
	bool onLineStatus; //在线状态，true：表示在线, false:表示离线

	u_uint64 upDataSize; //从前端传输到后端的数据量，单位字节
	u_uint64 downDataSize; //从后端读取的数据量, 单位字节
	u_uint64 sqlSize; //本客户端执行的sql语句的数量

	u_uint64 start_connect_time;//最近一次连接的开始时间，单位 为毫秒
	void clear() {
		memset(this, 0, sizeof(*this));
	}

	_client_query_info_part_t operator-(_client_query_info_part_t& a) {
		_client_query_info_part_t part;
		part.hashCode = this->hashCode;
		part.connectNum = this->connectNum - a.connectNum;
		part.connServerFail = this->connServerFail - a.connServerFail;
		part.queryNum = this->queryNum - a.queryNum;
		part.queryFailNum = this->queryFailNum - a.queryFailNum;
		part.trxNum = this->trxNum - a.trxNum;
		part.selectNum = this->selectNum - a.selectNum;
		part.insertNum = this->insertNum - a.insertNum;
		part.updateNum = this->updateNum - a.updateNum;
		part.deleteNum = this->deleteNum - a.deleteNum;
		part.onLineTime = this->onLineTime - a.onLineTime;
		part.onLineStatus = this->onLineStatus;
		part.upDataSize = this->upDataSize - a.upDataSize;
		part.downDataSize = this->downDataSize - a.downDataSize;
		part.sqlSize = this->sqlSize - a.sqlSize;
		part.start_connect_time = this->start_connect_time;
		return part;
	}
} ClientQueryInfoPart;

typedef struct _client_query_info_t{
	ClientQueryInfoPart part;
	std::string ipAddr;
	std::set<unsigned int> sqlList; //此客户端操作过的sql语句的hashcode.
	std::set<unsigned int> failSqlList; //失败的sql语句的hashcode.
	u_uint64 latest_connect_time; //最近一次登录时间的str格式，单位为:xxxx-xx-xx xx:xx:xx

	_client_query_info_t() {
		this->clear();
	}
	void clear() {
		this->ipAddr.clear();
		this->sqlList.clear();
		this->failSqlList.clear();
		this->part.clear();
		this->latest_connect_time = 0;
	}
	bool is_show() {
		if(this->part.hashCode == 0) {
			return false;
		} else {
			return true;
		}
	}

	_client_query_info_t& operator=(const _client_query_info_t& a) {
		this->part = a.part;
		this->ipAddr = a.ipAddr;
		this->latest_connect_time = a.latest_connect_time;
		this->sqlList.clear();
		this->failSqlList.clear();
		this->sqlList.insert(a.sqlList.begin(), a.sqlList.end());
		this->failSqlList.insert(a.failSqlList.begin(), a.failSqlList.end());
		return *this;
	}

	_client_query_info_t operator-(_client_query_info_t& a) {
		_client_query_info_t info;
		info.part = this->part - a.part;
		info.ipAddr = this->ipAddr;
		info.latest_connect_time = this->latest_connect_time;

		{
			std::set<unsigned int>::iterator it = this->sqlList.begin();
			for(; it != this->sqlList.end(); ++it) {
				std::set<unsigned int>::iterator sit = a.sqlList.find(*it);
				if (sit == a.sqlList.end()) {
					info.sqlList.insert(*it);
				}
			}
		}

		{
			std::set<unsigned int>::iterator it = this->failSqlList.begin();
			for(; it != this->failSqlList.end(); ++it) {
				std::set<unsigned int>::iterator sit = a.failSqlList.find(*it);
				if (sit == a.failSqlList.end()) {
					info.failSqlList.insert(*it);
				}
			}
		}
		return info;
	}
} ClientQueryInfo;


typedef struct _client_user_app_info_part_t{
	u_uint64 loginTimes;
	u_uint64 sqlcount; //临时使用，用于排序。
	_client_user_app_info_part_t operator-(_client_user_app_info_part_t& a) {
		_client_user_app_info_part_t part;
		part.loginTimes = this->loginTimes - a.loginTimes;
		part.sqlcount = 0;
		return part;
	}
}ClientUserAppInfoPart;
typedef struct _client_user_app_Info_t{
	ClientUserAppInfoPart part;
	unsigned int hashCode;
	std::string hostName;
	std::string userName;
	std::string appName;
	std::set<unsigned int> sqlList;//执行的sql语句
	std::string uniqueIdentify() {
		std::string result = hostName;
		result.append(userName);
		result.append(appName);
		return result;
	}
	bool is_show() {
		if(this->part.loginTimes <= 0) {
			return false;
		} else {
			return true;
		}
	}
	_client_user_app_Info_t& operator=(const _client_user_app_Info_t& a)
	{
		this->sqlList.clear();
		this->part = a.part;
		this->hashCode = a.hashCode;
		this->hostName = a.hostName;
		this->userName = a.userName;
		this->appName = a.appName;
		this->sqlList.clear();
		this->sqlList.insert(a.sqlList.begin(), a.sqlList.end());
		return *this;
	}
	_client_user_app_Info_t operator- (_client_user_app_Info_t& a) {
		_client_user_app_Info_t info;
		{
			std::set<unsigned int>::iterator it = this->sqlList.begin();
			for(; it != this->sqlList.end(); ++it) {
				std::set<unsigned int>::iterator sit = a.sqlList.find(*it);
				if (sit == a.sqlList.end()) {
					info.sqlList.insert(*it);
				}
			}
		}
		info.part = this->part - a.part;
		info.hostName = this->hostName;
		info.userName = this->userName;
		info.appName = this->appName;
		info.hashCode = this->hashCode;
		return info;
	}
} ClientUserAppInfo;

//sql信息的统计
typedef enum sql_op_t{
	sql_op_init = 0,
	sql_op_select = 1,
	sql_op_update = 2,
	sql_op_insert = 3,
	sql_op_replace = 4,
	sql_op_delete = 5,
	sql_op_commit = 6,
	sql_op_rollback = 7
}SqlOp;
typedef struct _sql_info_part_t{
	unsigned int hashCode;
	int audit;
	u_uint64 tabs;
	u_uint64 exec; //执行次数
	u_uint64 trans; //事务中执行的次数
	u_uint64 fail; //失败的次数
	u_uint64 execTime; //执行时间
	u_uint64 totalRow;//总行数
	u_uint64 recvSize;//接收的数据大小
	u_uint64 clientSetSize; //the clientSet size
	u_uint64 sqlSize; //sql 语句的长度
	SqlOp type;//1:select 2:update 3:insert 5:delete
	void clear() {
		memset(this, 0, sizeof(*this));
	}
	_sql_info_part_t operator- (_sql_info_part_t& a) {
		_sql_info_part_t part;

		part.hashCode = this->hashCode;
		part.audit = this->audit;
		part.tabs = this->tabs;
		part.exec = this->exec - a.exec;
		part.trans = this->trans - a.trans;
		part.fail = this->fail - a.fail;
		part.execTime = this->execTime - a.execTime;
		part.totalRow = this->totalRow - a.totalRow;
		part.recvSize = this->recvSize - a.recvSize;
		part.type = this->type;
		return part;
	}
}SqlInfoPart;
typedef struct sql_info_t{
	SqlInfoPart part;
	std::string sqlText;
	std::set<std::string> tableSet;
	std::set<unsigned int> clientSet;//保存所属客户端的hashcode

	sql_info_t() {
		this->clear();
	}
	void clear() {
		this->part.clear();
		this->sqlText.clear();
		this->tableSet.clear();
		this->clientSet.clear();
	}
	bool is_show() {
		if (this->part.hashCode == 0)
			return false;
		return true;
	}

	sql_info_t& operator=(const sql_info_t& a)
	{
		this->part = a.part;
		this->sqlText = a.sqlText;
		this->tableSet.clear();
		this->clientSet.clear();
		this->tableSet.insert(a.tableSet.begin(), a.tableSet.end());
		this->clientSet.insert(a.clientSet.begin(), a.clientSet.end());
		return *this;
	}
	sql_info_t operator-(sql_info_t& a) {
		sql_info_t info;
		info.part = this->part - a.part;
		info.sqlText = this->sqlText;

		{
			std::set<std::string>::iterator it = this->tableSet.begin();
			for (; it != this->tableSet.end(); ++it) {
				info.tableSet.insert(*it);
			}
		}

		{
			std::set<unsigned int>::iterator it = this->clientSet.begin();
			for (; it != this->clientSet.end(); ++it) {
				info.clientSet.insert(*it);
			}
		}
		return info;
	}
}SqlInfo;

//仅仅定义在此，不会直接通过此结构统计每张表的信息。当需要统计表的信息时临时存储
typedef struct _table_info_part_t{
	u_uint64 sqls;
	u_uint64 selectExec;//执行的总次数
	u_uint64 selectRows; //影响的总行数
	u_uint64 selectTime;//总时间
	u_uint64 insertExec;//执行的总次数
	u_uint64 insertRows; //影响的总行数
	u_uint64 insertTime;//总时间
	u_uint64 updateExec;//执行的总次数
	u_uint64 updateRows; //影响的总行数
	u_uint64 updateTime;//总时间
	u_uint64 deleteExec;//执行的总次数
	u_uint64 deleteRows; //影响的总行数
	u_uint64 deleteTime;//总时间
	void clear() {
		memset(this, 0, sizeof(*this));
	}
	_table_info_part_t operator- (_table_info_part_t& a) {
		_table_info_part_t info;
		info.sqls = this->sqls;
		info.selectExec = this->selectExec - a.selectExec;
		info.selectRows = this->selectRows - a.selectRows;
		info.selectTime = this->selectTime - a.selectTime;
		info.insertExec = this->insertExec - a.insertExec;
		info.insertRows = this->insertRows - a.insertRows;
		info.insertTime = this->insertTime - a.insertTime;
		info.updateExec = this->updateExec - a.updateExec;
		info.updateRows = this->updateRows - a.updateRows;
		info.updateTime = this->updateTime - a.updateTime;
		info.deleteExec = this->deleteExec - a.deleteExec;
		info.deleteRows = this->deleteRows - a.deleteRows;
		info.deleteTime = this->deleteTime - a.deleteTime;
		return info;
	}
}TableInfoPart;
typedef struct _table_info_t{
	TableInfoPart part;
	std::string tableName;
	_table_info_t() {
		this->clear();
	}
	void clear() {
		this->part.clear();
		this->tableName.clear();
	}
	bool is_show() {
		return true;
	}
	_table_info_t& operator= (const _table_info_t& a)
	{
		this->part = a.part;
		this->tableName = a.tableName;
		return *this;
	}
	_table_info_t operator- (_table_info_t& a) {
		_table_info_t info;
		info.part = this->part - a.part;
		info.tableName = this->tableName;
		return info;
	}
}TableInfo;


//仅仅定义在此处，程序中不会直接使用此结构，只在httpserver中使用一下
typedef struct _tables_map_info_part_t{
	u_uint64 exec;
	void clear() {
		memset(this, 0, sizeof(*this));
	}
	_tables_map_info_part_t operator-(_tables_map_info_part_t& a) {
		_tables_map_info_part_t part;
		part.exec = this->exec - a.exec;
		return part;
	}
}TableMapInfoPart;
typedef struct _tables_map_info_t{
	TableMapInfoPart part;
	std::string tablesName;
	std::set<unsigned int> sqlHashCode;
	_tables_map_info_t() {
		this->part.clear();
		this->sqlHashCode.clear();
	}
	bool is_show() {
		return true;
	}
	_tables_map_info_t& operator=(const _tables_map_info_t& a)
	{
		this->part = a.part;
		this->tablesName = a.tablesName;
		this->sqlHashCode.clear();
		this->sqlHashCode.insert(a.sqlHashCode.begin(), a.sqlHashCode.end());
		return *this;
	}
	_tables_map_info_t operator-(_tables_map_info_t& a) {
		_tables_map_info_t info;
		info.part = this->part - a.part;
		info.tablesName = this->tablesName;
		std::set<unsigned int>::iterator it = this->sqlHashCode.begin();
		for(; it != this->sqlHashCode.end(); ++it) {
			info.sqlHashCode.insert(*it);
		}
		return info;
	}
}TableMapInfo;


//统计事务信息
typedef struct _trans_info_part_t{
	unsigned int transHashCode;
	u_uint64 exec; //执行次数
	u_uint64 totalTime;//执行总时间
	u_uint64 rollbackTimes;//执行失败次数
	u_uint64 sqlNum; //sql语句条数
	u_uint64 maxTime; //执行事务的最大时间
	u_uint64 minTime; //执行事务的最小时间
	u_uint64 lastTime; //最后一次执行的时间长度
	void clear() {
		memset(this, 0, sizeof(*this));
	}
	_trans_info_part_t operator- (_trans_info_part_t& a) {
		_trans_info_part_t part;
		part.transHashCode = this->transHashCode;
		part.exec = this->exec - a.exec;
		part.totalTime = this->totalTime - a.totalTime;
		part.rollbackTimes = this->rollbackTimes - a.rollbackTimes;
		part.sqlNum = this->sqlNum - a.sqlNum;
		part.maxTime = this->maxTime;
		part.minTime = this->minTime;
		part.lastTime = this->lastTime;
		return part;
	}
}TransInfoPart;
typedef struct _trans_info_t{
	TransInfoPart part;
	std::set<unsigned int> sqlHashCode;
	u_uint64 lastTime; //事务最后执行的开始时间
	_trans_info_t() {
		this->lastTime = 0;
		this->clear();
	}
	void clear() {
		part.clear();
		sqlHashCode.clear();
	}
	bool is_show() {
		if (sqlHashCode.size() <= 0)
			return false;
		return true;
	}
	_trans_info_t& operator=(const _trans_info_t& a)
	{
		this->part = a.part;
		this->sqlHashCode.clear();
		this->sqlHashCode.insert(a.sqlHashCode.begin(), a.sqlHashCode.end());
		this->lastTime = a.lastTime;
		return *this;
	}
	_trans_info_t operator- (_trans_info_t& a) {
		_trans_info_t info;
		info.part = this->part - a.part;
		info.lastTime = this->lastTime;
		std::set<unsigned int>::iterator it = this->sqlHashCode.begin();
		for (; it != this->sqlHashCode.end(); ++it) {
			info.sqlHashCode.insert(*it);
		}
		return info;
	}
}TransInfo;
}
#endif /* STATS_RECORD_DEFINE_H_ */
