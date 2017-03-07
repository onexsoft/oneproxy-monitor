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
 * @FileName: transinfotab.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午3:26:06
 *  
 */

#ifndef STATS_DATABASE_TAB_TRANSINFOTAB_H_
#define STATS_DATABASE_TAB_TRANSINFOTAB_H_

#include "define.h"
#include "tabbase.h"

class TransInfoTabT: public TabBase{
public:
	TransInfoTabT(){
		this->m_transhashcode = 0;
		this->m_connhashcode = 0;
		this->m_exectime = 0;
	};
	virtual ~TransInfoTabT(){};
	virtual std::string createTableSql() {
		std::string ct =
				"create table if not exists transinfo("
				"connhashcode int4,"
				"transhashcode int4,"
				"exectime int4,"
				"starttime datetime, endtime datetime);";
		return ct;
	}

	virtual std::string insertDataSql(void* data) {
		assert(data);
		TransInfoTabT* titt = (TransInfoTabT*)data;
		std::stringstream ss;
		ss << "insert into transinfo(connhashcode, transhashcode, exectime, starttime, endtime)values(";
		ss << titt->get_connhashcode() << ", " << titt->get_transhashcode() << ", "
				<< titt->get_exectime() << ", '"
				<< this->tm2string(titt->get_starttime()) << "','"
				<< this->tm2string(titt->get_endtime()) << "');";

		return ss.str();
	}

	virtual std::string get_tableName() {
		return "transinfo";
	}
private:
	declare_class_member(unsigned int, connhashcode);
	declare_class_member(unsigned int, transhashcode);
	declare_class_member(unsigned int, exectime);
	declare_class_member(struct tm, starttime);
	declare_class_member(struct tm, endtime);
};

#endif /* STATS_DATABASE_TAB_TRANSINFOTAB_H_ */
