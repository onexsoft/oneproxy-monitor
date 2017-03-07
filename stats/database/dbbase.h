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
 * @FileName: dbbase.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年3月2日 下午3:43:12
 *  
 */

#ifndef STATS_DATABASE_DBBASE_H_
#define STATS_DATABASE_DBBASE_H_

#include "tabbase.h"

typedef enum _db_oper_t{
	DB_OPER_SUCCESS,
	DB_OPER_ERROR,
}DBOperT;

class DBBase {
public:
	DBBase();
	virtual ~DBBase();
	static DBBase* createDBObject();
	void destoryDBObject(DBBase* db);

	virtual DBOperT save_newClient(TabBase* tabBase) = 0;
	virtual DBOperT update_clientData(TabBase* tabBase) = 0;
	virtual DBOperT save_clientSqlRel(TabBase* tabBase) = 0;
	virtual DBOperT save_sqlExecRecord(TabBase* tabBase) = 0;
	virtual DBOperT save_newSql(TabBase* tabBase) = 0;
	virtual DBOperT save_newTable(TabBase* tabBase) = 0;
	virtual DBOperT save_sqlTableRel(TabBase* tabBase) = 0;
	virtual DBOperT update_sqlExec(TabBase* tabBase) = 0;
	virtual DBOperT save_transInfo(TabBase* tabBase) = 0;
	virtual DBOperT save_transSqlRel(TabBase* tabBase) = 0;
private:

};

#endif /* STATS_DATABASE_DBBASE_H_ */
