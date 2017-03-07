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
 * @FileName: sqlitedb.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午2:53:23
 *  
 */

#ifndef STATS_DATABASE_SQLITEDB_H_
#define STATS_DATABASE_SQLITEDB_H_

#include "sqlite3.h"
#include "define.h"
#include "dbbase.h"

class SqliteDB : public DBBase{
public:
	SqliteDB();
	SqliteDB(std::string dbPath);
	virtual ~SqliteDB();

	virtual DBOperT save_newClient(TabBase* tabBase);
	virtual DBOperT update_clientData(TabBase* tabBase);

	virtual DBOperT save_clientSqlRel(TabBase* tabBase);
	virtual DBOperT save_sqlExecRecord(TabBase* tabBase);
	virtual DBOperT save_newSql(TabBase* tabBase);
	virtual DBOperT save_newTable(TabBase* tabBase);
	virtual DBOperT save_sqlTableRel(TabBase* tabBase);
	virtual DBOperT update_sqlExec(TabBase* tabBase);
	virtual DBOperT save_transInfo(TabBase* tabBase);
	virtual DBOperT save_transSqlRel(TabBase* tabBase);

private:
	DBOperT open_sqlite();
	DBOperT create_tables();
	DBOperT is_existedRecord(std::string sql, int& result);
	static int judgeTableExisted(void* param, int nColumn, char** columnName, char** columnValue);
private:
	sqlite3* pDB;
	declare_class_member(std::string, dbPath);
};

#endif /* STATS_DATABASE_SQLITEDB_H_ */
