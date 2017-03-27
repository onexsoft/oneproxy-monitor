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
 * @FileName: sqlitedb.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午2:53:23
 *  
 */

#include "sqlitedb.h"
#include "logger.h"

#include "tabheader.h"

#define CHECK_OPEN_DB() do{\
	uif (this->pDB == NULL && DB_OPER_ERROR == this->open_sqlite()) {\
		logs(Logger::ERR, "open slite error");\
		return DB_OPER_ERROR;\
	}\
}while(0);

#define EXEC_SQL(SQL) do{\
	char* errmsg = NULL;\
	int res = sqlite3_exec(this->pDB, SQL.c_str(), NULL, NULL, &errmsg);\
	if (res != SQLITE_OK) {\
		logs(Logger::ERR, "execute sql(%s) error(%s)",SQL.c_str(), errmsg);\
		return DB_OPER_ERROR;\
	}\
}while(0);

SqliteDB::SqliteDB() {
	// TODO Auto-generated constructor stub
	this->pDB = NULL;
	this->open_sqlite();
}

SqliteDB::SqliteDB(std::string dbPath) {
	this->set_dbPath(dbPath);
	this->pDB = NULL;
	this->open_sqlite();
}

SqliteDB::~SqliteDB() {
	// TODO Auto-generated destructor stub
	if (pDB != NULL) {
		sqlite3_close(pDB);
	}
}

DBOperT SqliteDB::save_newClient(TabBase* tabBase) {
	CHECK_OPEN_DB();
	UserTabT* utt = (UserTabT*)tabBase;
	std::string is = utt->insertDataSql(utt);
	EXEC_SQL(is);
	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::update_clientData(TabBase* tabBase) {
	CHECK_OPEN_DB();
	UserTabT* utt = (UserTabT*)tabBase;
	if (utt->get_connHashCode() > 0) {
		std::string is = utt->updateData(utt);
		EXEC_SQL(is);
	}
	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::save_clientSqlRel(TabBase* tabBase) {
	CHECK_OPEN_DB();
	assert(tabBase);
	UserSqlRelTabT* usrtt = (UserSqlRelTabT*)tabBase;
	std::string is = usrtt->insertDataSql(usrtt);
	EXEC_SQL(is);
	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::save_sqlExecRecord(TabBase* tabBase) {
	CHECK_OPEN_DB();
	assert(tabBase);
	SqlExecTabT* sett = (SqlExecTabT*)tabBase;
	std::string is = sett->insertDataSql(sett);
	EXEC_SQL(is);

	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::save_newSql(TabBase* tabBase) {
	CHECK_OPEN_DB();
	assert(tabBase);
	SqlTabT* stt = (SqlTabT*)tabBase;

	//first judge the sql is exists or not.
	std::stringstream ss;
	ss << "select 1 from " << stt->get_tableName() << " where hashcode=" << stt->get_hashcode();
	std::string sql = ss.str();
	int isExisted = 0;
	if (SQLITE_OK != this->is_existedRecord(sql, isExisted)) {
		return DB_OPER_ERROR;
	}

	if (!isExisted) {
		sql = stt->insertDataSql(stt);
		EXEC_SQL(sql);
	}

	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::save_newTable(TabBase* tabBase) {
	CHECK_OPEN_DB();
	assert(tabBase);
	TableTabT *tt = (TableTabT*)tabBase;
	std::stringstream ss;
	ss << "select 1 from " << tt->get_tableName() << " where hashcode=" << tt->get_hashcode() << ";";
	std::string sql = ss.str();
	int isExisted = 0;
	if (SQLITE_OK != this->is_existedRecord(sql, isExisted)) {
		return DB_OPER_ERROR;
	}

	if (!isExisted) {
		sql = tt->insertDataSql(tt);
		EXEC_SQL(sql);
	}

	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::save_sqlTableRel(TabBase* tabBase) {

	CHECK_OPEN_DB();
	assert(tabBase);
	SqlTableRelTabT *strt = (SqlTableRelTabT*)tabBase;
	std::stringstream ss;
	ss << "select 1 from " << strt->get_tableName() << " where tabhc=" << strt->get_tabhc();
	ss << " and sqlhc=" << strt->get_sqlhc() << ";";
	std::string sql = ss.str();
	int isExisted = 0;
	if (SQLITE_OK != this->is_existedRecord(sql, isExisted)) {
		return DB_OPER_ERROR;
	}

	if (!isExisted) {
		sql = strt->insertDataSql(strt);
		EXEC_SQL(sql);
	}

	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::update_sqlExec(TabBase* tabBase)
{
	CHECK_OPEN_DB();
	assert(tabBase);
	SqlExecTabT* sett = (SqlExecTabT*)tabBase;

	std::stringstream ss;
	ss << "select 1 from " << sett->get_tableName() << " where connhashcode=" << sett->get_connHashCode();
	ss << " and exechashcode=" << sett->get_execHashcode() << ";";
	std::string sql = ss.str();

	int isExisted = 0;
	if (SQLITE_OK != this->is_existedRecord(sql, isExisted)) {
		return DB_OPER_ERROR;
	}

	if (isExisted) {
		sql = sett->updateDataSql(sett);
		EXEC_SQL(sql);
	}

	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::save_transInfo(TabBase* tabBase)
{
	CHECK_OPEN_DB();
	assert(tabBase);
	TransInfoTabT* titt = (TransInfoTabT*)tabBase;
	std::string is = titt->insertDataSql(titt);
	EXEC_SQL(is);

	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::save_transSqlRel(TabBase* tabBase) {
	CHECK_OPEN_DB();
	assert(tabBase);
	TransSqlRelTabT* tsrtt = (TransSqlRelTabT*)tabBase;
	std::stringstream ss;
	ss << "select 1 from " << tsrtt->get_tableName() << " where transhc=";
	ss << tsrtt->get_transhc() << " and sqlhc=" << tsrtt->get_sqlhc() << ";";

	std::string sql = ss.str();
	int isExisted = 0;
	if (SQLITE_OK != this->is_existedRecord(sql, isExisted)) {
		return DB_OPER_ERROR;
	}

	if (!isExisted) {
		sql = tsrtt->insertDataSql(tsrtt);
		EXEC_SQL(sql);
	}

	return DB_OPER_SUCCESS;
}


DBOperT SqliteDB::open_sqlite() {
	int res = sqlite3_open(this->get_dbPath().c_str(), &pDB);
	if (res != SQLITE_OK) {
		logs(Logger::ERR, "open db error(%s)", sqlite3_errmsg(pDB));
		this->pDB = NULL;
		return DB_OPER_ERROR;
	}

	//if table is not existed, create tables.
	this->create_tables();

	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::create_tables() {
	CHECK_OPEN_DB();

#define create_table(type) do{ \
		char *errmsg = NULL; \
		std::string csql; \
		std::string tableName; \
		type stt; \
		csql = stt.createTableSql(); \
		int res = sqlite3_exec(this->pDB, csql.c_str(), NULL, NULL, &errmsg); \
		if (res != SQLITE_OK) { \
			logs(Logger::ERR, "execute sql error(%s)", errmsg); \
			return DB_OPER_ERROR; \
		} \
}while(0);

	create_table(SqlTabT)
	create_table(SqlTableRelTabT)
	create_table(TableTabT)
	create_table(TransInfoTabT)
	create_table(TransSqlRelTabT)
	create_table(UserSqlRelTabT)
	create_table(UserTabT)
	create_table(SqlExecTabT);

#undef create_table

	return DB_OPER_SUCCESS;
}

DBOperT SqliteDB::is_existedRecord(std::string sql, int& result)
{
	char *errMsg = NULL;
	int res = sqlite3_exec(this->pDB, sql.c_str(), SqliteDB::judgeTableExisted, &result, &errMsg);
	if (res != SQLITE_OK) {
		logs(Logger::ERR, "execute sql (%s) error", sql.c_str());
		return DB_OPER_ERROR;
	}
	return DB_OPER_SUCCESS;
}

int SqliteDB::judgeTableExisted(void* param, int nColumn, char** columnName, char** columnValue)
{
	int* result = (int*)param;
	if (nColumn > 0) {
		*result = 1;
	} else {
		*result = 0;
	}

	return SQLITE_OK;
}
