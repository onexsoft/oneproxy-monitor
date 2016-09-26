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
#include <iostream>
#include <set>
#include "conf/config.h"

typedef enum _query_type_t{
	QUERY_TYPE_INIT,
	SIMPLE_QUERY_TYPE,
	TRANS_QUERY_TYPE,

	QUERY_TYPE_SUM
}QueryType;

typedef enum _translation_type_t{
	TRANS_NOTIN_TYPE, //非事务
	TRANS_BEGIN_TYPE,
	TRANS_COMMIT_TYPE,
	TRANS_ROLLBACK_TYPE,
}TranslationType;

typedef struct _doing_record_t{
	unsigned int sqlHashCode;//正在处理sql的hashCode
	std::string sqlText; //正在处理sql
	u_uint64 startQueryTime;
	u_uint64 totalRow; //当前执行sql返回的行数
//	std::string startQueryTimeStr;

	//for translation
	std::set<unsigned int> sqlSet; //the sql in trans
	u_uint64 totalTime; //事务的总格时间
	u_uint64 trans_start_time;//毫秒级别
//	time_t transStartTime; //事务的开始时间，秒级别
	bool rollback; //记录当前事务是否rollback
	QueryType type;
	TranslationType transFlag;//标记当前是不是在事务中

	_doing_record_t() {
		this->sqlHashCode = 0;
		this->startQueryTime = 0;
		this->totalRow = 0;
		this->totalTime = 0;
		this->trans_start_time = 0;
		this->rollback = false;
		type = QUERY_TYPE_INIT;
		transFlag = TRANS_NOTIN_TYPE;
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

class ProtocolBase;
typedef struct _connection_t{
	NetworkSocket *clins;
	NetworkSocket *servns;
	ProtocolBase* protocolBase;
	DataBase* database;
	DoingRecord record;

	u_uint64 createConnTime;//millsecond
	AutoPointer pointer;

	_connection_t() {
		this->clins = NULL;
		this->servns = NULL;
		this->protocolBase = NULL;
		this->database = NULL;
		this->createConnTime = 0;
	}
} Connection;

#endif /* CONNECTION_H_ */
