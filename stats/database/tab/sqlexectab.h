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
 * @FileName: sqlexectab.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年3月6日 下午3:29:33
 *  
 */

#ifndef STATS_DATABASE_TAB_SQLEXECTAB_H_
#define STATS_DATABASE_TAB_SQLEXECTAB_H_
#include "tabbase.h"
#include "define.h"
#include <sstream>

class SqlExecTabT : public TabBase {
public:
	SqlExecTabT(){
		this->m_connHashCode = 0;
		this->m_fails = 0;
		this->m_execHashcode = 0;
		this->m_recvsize = 0;
		this->m_rownum = 0;
		this->m_trans = 0;
	};
	virtual ~SqlExecTabT(){};

	virtual std::string createTableSql() {
		std::string ct = "create table if not exists sql_exec("
				"connhashcode int4,"
				"exechashcode int4,"
				"rownum int,"
				"querytime time,"
				"trans bool,"
				"fails bool,"
				"recvsize int);";
		return ct;
	}

	virtual std::string insertDataSql(void* data) {
		assert(data);
		SqlExecTabT* sett = (SqlExecTabT*)data;
		std::stringstream ss;
		ss << "insert into sql_exec(connhashcode, exechashcode, rownum, querytime, trans, fails, recvsize) values(";
		ss << sett->get_connHashCode() << ",";
		ss << sett->get_execHashcode() << ",";
		ss << sett->get_rownum() << ",";
		ss << "'" << this->tm2string(sett->get_querytime()) << "',";
		ss << sett->get_trans() << ",";
		ss << sett->get_fails() << ",";
		ss << sett->get_recvsize() << ");";
		return ss.str();
	}

	virtual std::string updateDataSql(void* data) {
		assert(data);
		SqlExecTabT* sett = (SqlExecTabT*)data;
		std::stringstream ss;
		ss << " update " << get_tableName() << " set rownum = rownum + " << sett->get_rownum();
		ss << " , trans = trans + " << sett->get_trans() << ", fails = fails + " << sett->get_fails();
		ss << " , recvsize = recvsize + " << sett->get_recvsize() ;
		ss << " where connhashcode=" << sett->get_connHashCode();
		ss << " and exechashcode=" << sett->get_execHashcode() << ";";

		return ss.str();
	}

	virtual std::string get_tableName() {
		return "sql_exec";
	}
private:
	declare_class_member(unsigned int , connHashCode)//connection hashcode .
	declare_class_member(unsigned int, execHashcode);//sql hashcode hash(sql)
	declare_class_member(int, rownum); //the sql query result
	declare_class_member(struct tm, querytime);
	declare_class_member(bool, trans);//true: Execute in the translation; or false.
	declare_class_member(bool, fails);//true: Execute fails, or success.
	declare_class_member(int, recvsize);//the result's size of execute sql
};

#endif /* STATS_DATABASE_TAB_SQLEXECTAB_H_ */
