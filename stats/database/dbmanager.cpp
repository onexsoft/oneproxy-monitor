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
 * @FileName: dbmanager.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年3月2日 上午9:59:15
 *  
 */

#include "dbmanager.h"
#include "systemapi.h"
#include "tabheader.h"
#include "tool.h"

DBManager::DBManager()
	:ThreadTask<DBDataT>(thread_type_db, "dbmanager", DBManager::start, (void*)this){
	// TODO Auto-generated constructor stub
	dbBase = DBBase::createDBObject();
}

DBManager::~DBManager() {
	// TODO Auto-generated destructor stub
	if (dbBase) {
		dbBase->destoryDBObject(dbBase);
	}
}

void DBManager::start(void* data, void* args) {
	assert(data);
	assert(args);
	DBDataT dbt = (DBDataT)(*((DBDataT*)data));
	DBManager* dbm = (DBManager*)args;
	switch (dbt.type) {
	case DB_DATA_TYPE_CLIENT_INFO:
	{
		ClientInfoT* cit = (ClientInfoT*)dbt.data;
		if (dbm->dbBase && cit) {
			UserTabT utt;
			utt.set_connHashCode(cit->connHashCode);
			utt.set_caddr(cit->clientIp);
			utt.set_conndatetime(dbm->millisecond2tm(cit->start_connect_time));
			utt.set_cport(cit->cport);
			utt.set_hashcode(cit->hashcode);
			utt.set_recvdata(0);
			utt.set_saddr(cit->serverIp);
			utt.set_senddata(0);
			utt.set_sport(cit->sport);

			dbm->dbBase->save_newClient(&utt);
		}
		if (cit) {
			cit->free_instance();
		}
	}
		break;
	case DB_DATA_TYPE_CLIENT_INFO_UPDATE:
	{
		ClientInfoT* cit = (ClientInfoT*)dbt.data;
		if (dbm->dbBase && cit) {
			UserTabT utt;
			utt.set_connHashCode(cit->connHashCode);
			utt.set_recvdata(cit->recvSize);
			utt.set_senddata(cit->sendSize);

			dbm->dbBase->update_clientData(&utt);
		}
		if (cit) {
			cit->free_instance();
		}
	}
	break;

	case DB_DATA_TYPE_CLIENT_ADD_SQL:
	{
		ClientSqlRetS* usrs = (ClientSqlRetS*)dbt.data;
		if (dbm->dbBase && usrs) {
			UserSqlRelTabT usrtt;
			usrtt.set_sqlexechc(usrs->sqlexec_hashcode);
			usrtt.set_sqlhc(usrs->sql_hashcode);
			usrtt.set_connhc(usrs->conn_hashcode);
			usrtt.set_exectime(*SystemApi::system_timeTM());
			dbm->dbBase->save_clientSqlRel(&usrtt);
		}
		if (usrs) {
			usrs->free_instance();
		}
	}
	break;
	case DB_DATA_TYPE_CLIENT_EXEC_SQL:
	{
		SqlInfoS* sis = (SqlInfoS*)dbt.data;
		if (dbm->dbBase && sis) {
			//add exec record to sql_exec table.
			SqlExecTabT sett;
			sett.set_connHashCode(sis->connHashCode);
			sett.set_fails(0);
			sett.set_execHashcode(sis->sqlExecHashCode);
			sett.set_querytime(dbm->millisecond2tm(sis->execTime));
			sett.set_recvsize(0);
			sett.set_rownum(0);
			sett.set_trans(false);
			dbm->dbBase->save_sqlExecRecord(&sett);

			//add sql to sql table.
			SqlTabT stt;
			stt.set_hashcode(sis->sqlHashCode);
			stt.set_sql(sis->sql);
			stt.set_tabs(sis->tabCnt);
			stt.set_sqlType(sis->queryType);
			dbm->dbBase->save_newSql(&stt);

			unsigned int size = sis->tabNameVec.size();
			for(unsigned int i = 0; i < size; ++i) {
				//add table name to table
				TableTabT tt;
				std::string tn = sis->tabNameVec.at(i);
				tt.set_hashcode(Tool::quick_hash_code(tn.c_str(), tn.length()));
				tt.set_tabName(tn);
				dbm->dbBase->save_newTable(&tt);

				//save sql and table relation to db.
				SqlTableRelTabT strt;
				strt.set_sqlhc(sis->sqlHashCode);
				strt.set_tabhc(tt.get_hashcode());
				dbm->dbBase->save_sqlTableRel(&strt);
			}
		}
		if (sis) {
			sis->free_instance();
		}
	}
	break;
	case DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_DATASIZE:
	case DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_TRANS:
	case DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_ROWNUM:
	case DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_FAIL:
	{
		SqlExecS* ses = (SqlExecS*)dbt.data;
		if (dbm->dbBase && ses) {
			SqlExecTabT sett;
			sett.set_connHashCode(ses->connHashCode);
			sett.set_execHashcode(ses->sqlExecHashCode);
			sett.set_fails(0);
			sett.set_recvsize(0);
			sett.set_rownum(0);
			sett.set_trans(0);

			switch (dbt.type) {
			case DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_DATASIZE:
			{
				sett.set_recvsize(ses->recvSize);
			}
			break;
			case DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_TRANS:
			{
				sett.set_trans(ses->inTrans);
			}
			break;
			case DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_ROWNUM:
			{
				sett.set_rownum(ses->rowNum);
			}
			break;
			case DB_DATA_TYPE_CLIENT_EXEC_SQL_UPDATE_FAIL:
			{
				sett.set_fails(ses->fail);
			}
			break;
			default:
				break;
			}
			dbm->dbBase->update_sqlExec(&sett);
		}
		if (ses) {
			ses->free_instance();
		}
	}
	break;
	case DB_DATA_TYPE_CLIENT_EXEC_TRANS:
	{
		TransInfoS* tis = (TransInfoS*)dbt.data;
		if (dbm->dbBase && tis) {
			//add trans info to trans info table
			TransInfoTabT titt;
			titt.set_connhashcode(tis->connHashCode);
			titt.set_transhashcode(tis->transHashCode);
			titt.set_starttime(dbm->millisecond2tm(tis->start_time));
			titt.set_endtime(dbm->millisecond2tm(tis->end_time));
			titt.set_exectime(tis->start_time - tis->end_time);
			dbm->dbBase->save_transInfo(&titt);

			unsigned int size = tis->sqlVec.size();
			for(unsigned int i = 0; i < size; ++i) {
				unsigned int sqlHashCode = tis->sqlVec.at(i);
				TransSqlRelTabT tsrtt;
				tsrtt.set_sqlhc(sqlHashCode);
				tsrtt.set_transhc(tis->transHashCode);
				dbm->dbBase->save_transSqlRel(&tsrtt);
			}
		}
		if (tis) {
			tis->free_instance();
		}
	}
	break;
	default:
		break;
	}
}

struct tm DBManager::millisecond2tm(u_uint64 mst)
{
	time_t t = mst / 1000;
	struct tm tm = {0};
	#ifdef linux
		localtime_r(&t, &tm);
	#else
		tm = *localtime(&t);
	#endif
	return tm;
}
