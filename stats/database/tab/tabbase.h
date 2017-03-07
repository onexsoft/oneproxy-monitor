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
 * @FileName: tabbase.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月28日 下午4:15:48
 *  
 */

#ifndef STATS_DATABASE_TAB_TABBASE_H_
#define STATS_DATABASE_TAB_TABBASE_H_
#include <time.h>
#include <iostream>
#include <string>
#include <string.h>
class TabBase{
public:
	TabBase(){};
	virtual ~TabBase(){};
	virtual std::string createTableSql() = 0;
	virtual std::string insertDataSql(void* data) = 0;
	virtual std::string get_tableName() = 0;
	std::string tm2string(struct tm tm_t) {
		char buf[32] = {0};
		strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tm_t);
		return std::string(buf, strlen(buf));
	}
};
#endif /* STATS_DATABASE_TAB_TABBASE_H_ */
