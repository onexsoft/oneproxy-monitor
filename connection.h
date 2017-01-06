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
* @FileName: connection.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月18日
*
*/

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "networksocket.h"
#include "record_define.h"
#include "conf/config.h"
#include "handlemanager.h"
#include <iostream>
#include <set>
#include <map>

typedef enum _query_type_t{
	QUERY_TYPE_INIT,

	SIMPLE_QUERY_TYPE,
	SIMPLE_QUERY_SELECT_TYPE,
	SIMPLE_QUERY_SUM,

	TRANS_QUERY_TYPE,
	TRANS_QUERY_TRANSMGRREQ_BEGIN_TYPE,
	TRANS_QUERY_TRANSMGRREQ_COMMIT_TYPE,
	TRANS_QUERY_TRANSMGRREQ_ROLLBACK_TYPE,
	TRANS_QUERY_TRANS_ON_TYPE,
	TRANS_QUERY_TRANS_OFF_TYPE,
	TRANS_QUERY_TRANS_ON_COMMIT_TYPE,
	TRANS_QUERY_TRANS_ON_ROLLBACK_TYPE,
	TRANS_QUERY_BEGIN_TYPE,
	TRANS_QUERY_COMMIT_TYPE,
	TRANS_QUERY_ROLLBACK_TYPE,
	TRANS_QUERY_FINISHED_TYPE,//finished trans.
	TRANS_QUERY_SUM,

	QUERY_TYPE_SUM,
} QueryType;

typedef enum _conn_exec_status_t{
	CONN_EXEC_STATUS_NORMAL,
	CONN_EXEC_STATUS_ERROR//when connection get error packet from server, set this status.
}ConnExecStatus;

typedef struct _sql_info_t{
	unsigned int sqlHashCode; //sql hash code
	std::string sqlText; // sqlparse modifyed sql.
	unsigned int tableCount;//the table count in sql text.
	stats::SqlOp queryType; //query type
	std::vector<std::string> tableNameVec; //table name vector
	_sql_info_t() {
		this->sqlHashCode = 0;
		this->tableCount = 0;
		this->queryType = stats::sql_op_init;
	}
} SqlInfo;

typedef struct _doing_record_t{
	SqlInfo sqlInfo;//current sql info.

	u_uint64 startQueryTime;
	u_uint64 totalRow; //当前执行sql返回的行数

	//for translation
	std::set<unsigned int> sqlSet; //the sql in trans
	u_uint64 totalTime; //事务的总共时间
	u_uint64 trans_start_time;//毫秒级别
	bool rollback; //记录当前事务是否rollback
	QueryType type;

	_doing_record_t() {
		this->startQueryTime = 0;
		this->totalRow = 0;
		this->totalTime = 0;
		this->trans_start_time = 0;
		this->rollback = false;
		type = QUERY_TYPE_INIT;
	}
}DoingRecord;

typedef struct _auto_pointer_t {
private:
	void* data;
	typedef void (*Func)(void* data);
	Func func;
public:
	_auto_pointer_t() {
		this->data = NULL;
		this->func = NULL;
	}
	~_auto_pointer_t() {
		if (this->data != NULL && this->func != NULL) {
			this->func(this->data);
			this->data = NULL;
		}
	}
	void set_autoPointer(void* data, Func func) {
		this->data = data;
		this->func = func;
	}
	void* get_data() {
		return data;
	}
}AutoPointer;

typedef struct _database_in_connect{
	DataBaseGroup* dataBaseGroup;
	DataBase* masterDataBase;
	DataBase* slaveDataBase;
	DataBase* currentDataBase;//要么指向masterDataBase要么指向slaveDataBase
	_database_in_connect() {
		this->dataBaseGroup = NULL;
		this->masterDataBase = NULL;
		this->slaveDataBase = NULL;
		this->currentDataBase = NULL;
	}
} ConnDB;

typedef struct _socket_set_t{
	NetworkSocket* curclins;
	NetworkSocket* masters;
	NetworkSocket* slavers;
	NetworkSocket* curservs;//指向masters或者slavers
	_socket_set_t() {
		this->curclins = NULL;
		this->curservs = NULL;
		this->masters = NULL;
		this->slavers = NULL;
	}
	~_socket_set_t() {
		this->curclins = NULL;
		this->curservs = NULL;
		this->masters = NULL;
		this->slavers = NULL;
	}

	NetworkSocket* get_clientSock() {
		return this->curclins;
	}

	NetworkSocket* get_curServSock() {
		return this->curservs;
	}

	void use_masterServer() {
		this->curservs = masters;
	}

	void use_slaverServer() {
		this->curservs = this->slavers;
	}
} SocketSet;

typedef struct _session_data_t{
	HandleManager preparedCursorManager;
	stats::SqlInfo* sqlInfo;
	stats::ClientQueryInfo* clientInfo;
	_session_data_t() {
		this->sqlInfo = NULL;
		this->clientInfo = NULL;
	}
} SessionData;

class ProtocolBase;
typedef struct _connection_t {
	SocketSet sock;
	ProtocolBase* protocolBase;
	ConnDB database;
	DoingRecord record;
	u_uint64 createConnTime;//millsecond
	u_uint64 activeTime; //when receive data, update the time. unit:second.
	AutoPointer pointer;
	SessionData sessData;
	ConnExecStatus status;
	_connection_t() {
		this->protocolBase = NULL;
		this->createConnTime = 0;
		this->activeTime = 0;
		this->status = CONN_EXEC_STATUS_NORMAL;
	}
#define clins() sock.curclins
#define servns() sock.curservs
#define curdb() database.currentDataBase
#define handleManager() sessData.preparedCursorManager
} Connection;

typedef enum _conn_finish_type{
	CONN_FINISHED_NORMAL,
	CONN_FINISHED_ERR,
	CONN_FINISHED_FRONT_CLOSE//add by huih@20170105
}ConnFinishType;

#endif /* CONNECTION_H_ */
