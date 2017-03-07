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
 * @FileName: usertab.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午3:22:48
 *  
 */

#ifndef STATS_DATABASE_TAB_USERTAB_H_
#define STATS_DATABASE_TAB_USERTAB_H_

#include "define.h"
#include "tabbase.h"

class UserTabT: public TabBase {
public:
	UserTabT() {
		this->m_connHashCode = 0;
		this->m_cport = 0;
		this->m_hashcode = 0;
		this->m_recvdata = 0;
		this->m_senddata = 0;
		this->m_sport = 0;
	};
	virtual ~UserTabT(){};

	virtual std::string createTableSql() {
			std::string ct = "create table if not exists user("
					"connHashCode int4,"
					"hashcode int4,"
					"caddr varchar(20),"
					"cport int,"
					"saddr varchar(20),"
					"sport int,"
					"conndatetime datetime,"
					"senddata bigint,"
					"recvdata bigint);";
			return ct;
	}

	virtual std::string insertDataSql(void* data) {
		assert(data);
		UserTabT* utt = (UserTabT*)data;
		std::stringstream ss;
		ss << "insert into user(connHashCode, hashcode, caddr, cport, saddr, sport, conndatetime, senddata, recvdata) values(";
		ss << utt->get_connHashCode() << ",";
		ss << utt->get_hashcode() << "," ;
		ss << "'" << utt->get_caddr() << "'," << utt->get_cport() << ",";
		ss << "'" << utt->get_saddr() << "'," << utt->get_sport() << ",";
		ss << "'" << this->tm2string(utt->get_conndatetime()) << "'," << utt->get_senddata() << ",";
		ss << utt->get_recvdata() << ");";
		return ss.str();
	}

	std::string updateData(UserTabT* utt) {
		assert(utt);
		std::stringstream ss;
		ss << "update user set senddata = senddata + " << utt->get_senddata();
		ss << ", recvdata = recvdata + " << utt->get_recvdata();
		ss << " where connHashCode=" << utt->get_connHashCode() << ";";
		return ss.str();
	}

	virtual std::string get_tableName() {
		return "user";
	}
private:
	declare_class_member(unsigned int, connHashCode);
	declare_class_member(unsigned int, hashcode);
	declare_class_member(std::string, caddr);
	declare_class_member(int, cport);
	declare_class_member(std::string, saddr);
	declare_class_member(int, sport);
	declare_class_member(struct tm, conndatetime);
	declare_class_member(u_uint64, senddata);
	declare_class_member(u_uint64, recvdata);
};
#endif /* STATS_DATABASE_TAB_USERTAB_H_ */
