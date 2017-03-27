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
 * @FileName: dbbase.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年3月2日 下午3:43:12
 *  
 */

#include "dbbase.h"
#include "sqlitedb.h"
#include "logger.h"

DBBase::DBBase() {
	// TODO Auto-generated constructor stub

}

DBBase::~DBBase() {
	// TODO Auto-generated destructor stub
}

DBBase* DBBase::createDBObject() {
	SqliteDB* sdb = new SqliteDB(std::string("./monitor.db"));
	if (sdb == NULL) {
		logs(Logger::ERR, "open sqlite db error");
		return NULL;
	}
	return (DBBase*)sdb;
}

void DBBase::destoryDBObject(DBBase* db) {
	SqliteDB* sdb = (SqliteDB*)db;
	if (sdb) {
		delete sdb;
		sdb = NULL;
	}
}
