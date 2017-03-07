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
 * @FileName: sqltab.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午3:23:21
 *  
 */

#ifndef STATS_DATABASE_TAB_SQLTAB_H_
#define STATS_DATABASE_TAB_SQLTAB_H_

#include "tabbase.h"
#include "define.h"
#include <sstream>

class SqlTabT : public TabBase {
public:
	SqlTabT() {
		this->m_hashcode = 0;
		this->m_tabs = 0;
		this->m_sqlType = 0;
	};
	virtual ~SqlTabT(){};

	virtual std::string createTableSql() {
		std::string ct = "create table if not exists sql("
				"hashcode int4,"
				"sql text,"
				"tabs int,"
				"sqlType int"
				");";
		return ct;
	}

	virtual std::string insertDataSql(void* data) {
		assert(data);
		SqlTabT* stt = (SqlTabT*)data;
		std::stringstream ss;
		ss << "insert into sql(hashcode, sql, tabs, sqltype) values(";
		ss << stt->get_hashcode() << ",'";
		ss << stt->get_sql() << "',";
		ss << stt->get_tabs() << ",";
		ss << stt->get_sqlType() << ");";
		return ss.str();
	}

	virtual std::string get_tableName() {
		return "sql";
	}
private:
	declare_class_member(unsigned int, hashcode);//sql hashcode hash(sql)
	declare_class_member(std::string, sql);
	declare_class_member(int, tabs);//the table of sql
	declare_class_member(int, sqlType);//insert, select, udpate, delete.
};
#endif /* STATS_DATABASE_TAB_SQLTAB_H_ */
