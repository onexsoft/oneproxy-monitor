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
 * @FileName: FreeImpl.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年12月7日 下午3:05:08
 *  
 */

#include "freeimpl.h"

FreeImpl::FreeImpl() {
	// TODO Auto-generated constructor stub

}

FreeImpl::~FreeImpl() {
	// TODO Auto-generated destructor stub
}

int FreeImpl::get_databaseFromGroup(Connection& con)
{
	uif(con.database.dataBaseGroup == NULL)
		return -1;

	if (con.database.dataBaseGroup->get_dbMasterGroupVec().size() > 0) {
		con.database.masterDataBase = con.database.dataBaseGroup->get_dbMasterGroupVec().front();
	} else {//必须需要master
		logs(Logger::ERR, "no master database in config file");
		return -1;
	}

	if (con.database.dataBaseGroup->get_dbSlaveGroupVec().size() > 0) {//可以没有slave
		con.database.slaveDataBase = con.database.dataBaseGroup->get_dbSlaveGroupVec().front();
	}

	//first set current database is master database
	con.database.currentDataBase = con.database.masterDataBase;

	return 0;
}
