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
 * @FileName: sqltablereltab.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午3:25:13
 *  
 */

#ifndef STATS_DATABASE_TAB_SQLTABLERELTAB_H_
#define STATS_DATABASE_TAB_SQLTABLERELTAB_H_

#include "tabbase.h"
#include "define.h"

class SqlTableRelTabT: public TabBase{
public:
	SqlTableRelTabT() {
		this->m_sqlhc = 0;
		this->m_tabhc = 0;
	};
	virtual ~SqlTableRelTabT() {};

	virtual std::string createTableSql() {
		std::string ct =
				"create table if not exists sql_table_rel("
				"sqlhc int4, tabhc int4);";
		return ct;
	}

	virtual std::string insertDataSql(void* data) {
		assert(data);
		SqlTableRelTabT* strt = (SqlTableRelTabT*)data;
		std::stringstream ss;
		ss << "insert into sql_table_rel(sqlhc, tabhc)values(";
		ss << strt->get_sqlhc() << ",";
		ss << strt->get_tabhc() << ");";

		return ss.str();
	}

	virtual std::string get_tableName() {
		return "sql_table_rel";
	}
private:
	declare_class_member(unsigned int, sqlhc);
	declare_class_member(unsigned int, tabhc);
};
#endif /* STATS_DATABASE_TAB_SQLTABLERELTAB_H_ */
