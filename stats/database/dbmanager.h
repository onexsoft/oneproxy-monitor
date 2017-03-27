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
 * @FileName: dbmanager.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年3月2日 上午9:59:15
 *  
 */

#ifndef STATS_DBMANAGER_H_
#define STATS_DBMANAGER_H_

#include "define.h"
#include "db_define.h"
#include "threadtask.h"
#include "dbbase.h"

class DBManager : public ThreadTask<DBDataT>{
private:
	DBManager();
public:
	static DBManager* instance();
	virtual ~DBManager();
	virtual int init_childThread();
private:
	static void start(void* data, void* args);
	struct tm millisecond2tm(u_uint64 mst);
private:
	DBBase *dbBase;
	static DBManager* pDBManager;
	static MutexLock lock;
};

#endif /* STATS_DBMANAGER_H_ */
