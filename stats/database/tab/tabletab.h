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
 * @FileName: tabletab.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午3:24:38
 *  
 */

#ifndef STATS_DATABASE_TAB_TABLETAB_H_
#define STATS_DATABASE_TAB_TABLETAB_H_

#include "tabbase.h"
#include "define.h"

class TableTabT: public TabBase {
public:
	TableTabT() {
		this->m_hashcode = 0;
	};
	virtual ~TableTabT(){};

	virtual std::string createTableSql() {
		std::string ct =
				"create table if not exists sqltable("
				"hashcode int4, tabname text);";
		return ct;
	}

	virtual std::string insertDataSql(void* data) {
		assert(data);
		TableTabT* ttt = (TableTabT*)data;
		std::stringstream ss;
		ss << "insert into sqltable(hashcode, tabname) values(";
		ss << ttt->get_hashcode() << ", '" << ttt->get_tabName() << "');";
		return ss.str();
	}

	virtual std::string get_tableName() {
		return "sqltable";
	}
private:
	declare_class_member(unsigned int, hashcode);
	declare_class_member(std::string, tabName);
};

#endif /* STATS_DATABASE_TAB_TABLETAB_H_ */
