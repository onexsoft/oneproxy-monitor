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
 * @FileName: db_define.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年3月2日 上午11:30:46
 *  
 */

#ifndef STATS_DATABASE_DB_DEFINE_H_
#define STATS_DATABASE_DB_DEFINE_H_

#include <iostream>
#include <list>
#include <vector>

typedef enum _db_data_type_t{
	DB_DATA_TYPE_INIT,
	DB_DATA_TYPE_CLIENT_INFO,
	DB_DATA_TYPE_CLIENT_INFO_UPDATE,
	DB_DATA_TYPE_CLIENT_ADD_SQL,
	DB_DATA_TYPE_CLIENT_EXEC_SQL,
	DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_DATASIZE,
	DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_TRANS,
	DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_ROWNUM,
	DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_FAIL,
	DB_DATA_TYPE_CLIENT_EXEC_TRANS,
}DBDataTypeT;

typedef struct db_data_t{
	DBDataTypeT type;
	void* data;
	db_data_t() {
		type = DB_DATA_TYPE_INIT,
		data = NULL;
	}
}DBDataT;
typedef std::list<DBDataT> DBDataList;

#define create_instance_macro(type) \
	static type* create_instance() {\
		return new type();\
	}
#define free_instance_macro() \
		void free_instance() {\
			delete this;\
		}
#define add_instance_func(type) create_instance_macro(type)\
	free_instance_macro()


typedef struct _client_info_t{
	u_uint32 connHashCode;
	std::string clientIp;
	int cport;
	std::string serverIp;
	int sport;
	u_uint32 hashcode;
	u_uint64 start_connect_time;
	u_uint64 sendSize;
	u_uint64 recvSize;
	add_instance_func(_client_info_t);
} ClientInfoT;

typedef struct _client_sql_ret_t{
	u_uint32 conn_hashcode;
	u_uint32 sql_hashcode;
	u_uint32 sqlexec_hashcode;
	add_instance_func(_client_sql_ret_t);
}ClientSqlRetS;

typedef struct _sql_info_s{
	u_uint32 connHashCode;
	u_uint32 sqlExecHashCode;
	u_uint32 sqlHashCode;
	std::string sql;
	int tabCnt;
	int queryType;
	u_uint64 execTime;
	std::vector<std::string> tabNameVec;
	add_instance_func(_sql_info_s);
}SqlInfoS;

typedef struct _sql_exec_s{
	u_uint32 sqlExecHashCode;
	u_uint32 connHashCode;
	int recvSize;
	int fail;
	int inTrans;
	int rowNum;
	add_instance_func(_sql_exec_s);
}SqlExecS;

typedef struct _trans_info_s{
	u_uint32 connHashCode;
	u_uint32 transHashCode;
	u_uint64 start_time;
	u_uint64 end_time;
	std::vector<u_uint32> sqlVec;
	add_instance_func(_trans_info_s);
}TransInfoS;

#endif /* STATS_DATABASE_DB_DEFINE_H_ */
