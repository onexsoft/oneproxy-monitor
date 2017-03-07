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
 * @FileName: usersqlreltab.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午3:24:01
 *  
 */

#ifndef STATS_DATABASE_TAB_USERSQLRELTAB_H_
#define STATS_DATABASE_TAB_USERSQLRELTAB_H_

#include "define.h"
#include "tabbase.h"

class UserSqlRelTabT: public TabBase{
public:
	UserSqlRelTabT(){
		this->m_sqlhc = 0;
		this->m_connhc = 0;
		this->m_sqlexechc = 0;
	};
	virtual ~UserSqlRelTabT(){};

	virtual std::string createTableSql() {
		std::string ct =
				"create table if not exists user_sql_rel("
				"connhc int4, sqlexechc int4, sqlhc int4,"
				"exectime datetime);";
		return ct;
	}

	virtual std::string insertDataSql(void* data) {
		assert(data);
		UserSqlRelTabT* usrtt = (UserSqlRelTabT*)data;

		std::stringstream ss;
		ss << "insert into user_sql_rel(connhc, sqlexechc, sqlhc, exectime) values(";
		ss << usrtt->get_connhc() << "," << usrtt->get_sqlexechc() << ",";
		ss << usrtt->get_sqlhc() << ",'"<< this->tm2string(usrtt->get_exectime()) << "');";

		return ss.str();
	}

	virtual std::string get_tableName() {
		return "user_sql_rel";
	}
private:
	declare_class_member(unsigned int, connhc);
	declare_class_member(unsigned int, sqlexechc);
	declare_class_member(unsigned int, sqlhc);
	declare_class_member(struct tm, exectime);
};

#endif /* STATS_DATABASE_TAB_USERSQLRELTAB_H_ */
