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
 * @FileName: transsqlreltab.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午3:26:53
 *  
 */

#ifndef STATS_DATABASE_TAB_TRANSSQLRELTAB_H_
#define STATS_DATABASE_TAB_TRANSSQLRELTAB_H_

#include "tabbase.h"
#include "define.h"

class TransSqlRelTabT: public TabBase {
public:
	TransSqlRelTabT() {
		this->m_sqlhc = 0;
		this->m_transhc = 0;
	};
	virtual ~TransSqlRelTabT(){};

	virtual std::string createTableSql() {
		std::string ct = "create table if not exists trans_sql_rel("
				"transhc int4, sqlhc int4);";
		return ct;
	}

	virtual std::string insertDataSql(void* data) {
		std::stringstream ss;
		TransSqlRelTabT *tsrtt = (TransSqlRelTabT*)data;
		ss << "insert into trans_sql_rel(transhc, sqlhc)values(";
		ss << tsrtt->get_transhc() << "," << tsrtt->get_sqlhc() << ");";
		return ss.str();
	}

	virtual std::string get_tableName() {
		return "trans_sql_rel";
	}
private:
	declare_class_member(unsigned int, transhc);
	declare_class_member(unsigned int, sqlhc);
};
#endif /* STATS_DATABASE_TAB_TRANSSQLRELTAB_H_ */
