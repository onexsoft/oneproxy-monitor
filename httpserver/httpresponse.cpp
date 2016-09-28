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
* @FileName: httpresponse.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月15日
*
*/

#include "httpresponse.h"
#include "httphtml.h"
#include "record.h"
#include "machinestatus.h"
#include <stdio.h>
#include <algorithm>
#include <stddef.h>
#include <sstream>
#include "systemapi.h"
#include "tool.h"

#ifdef linux
#define offsetof(TYPE, MEMBER) __builtin_offsetof (TYPE, MEMBER)
#endif
#define order_func(fieldType, structType, field, asc) &HttpResponse::compare<fieldType, (int)offsetof(structType, field), asc>

HttpResponse::HttpResponse()
{
	this->response_setMenu(this->topMenu);

	this->responseFuncMap["/topclients"] = &HttpResponse::response_topclients;
	this->responseFuncMap["/topsqls"] = &HttpResponse::response_topsqls;
	this->responseFuncMap["/toptables"] = &HttpResponse::response_toptables;
	this->responseFuncMap["/toptablesmap"] = &HttpResponse::response_toptablesMap;
	this->responseFuncMap["/toptrans"] = &HttpResponse::response_topTrans;
	this->responseFuncMap["/findsql"] = &HttpResponse::response_findsql;

	this->responseFuncMap["/home"] = &HttpResponse::response_home;
	this->responseFuncMap["/setting"] = &HttpResponse::response_setting;
	this->responseFuncMap["/savesetting"] = &HttpResponse::response_saveSetting;
	this->responseFuncMap["/rsstats"] = &HttpResponse::response_reset;

	this->responseFuncMap["/task"] = &HttpResponse::response_task;
	this->responseFuncMap["/taskthread"] = &HttpResponse::response_taskThread;
	this->responseFuncMap["/lock"] = &HttpResponse::response_lock;
	this->responseFuncMap["/htmlpage"] = &HttpResponse::response_htmlPage;
}

HttpResponse::~HttpResponse()
{
	this->clear_topMenu();
}

void HttpResponse::response_setMenu(TopMenu& topMenu)
{
	static Menu_t menu[] = {
		add_topMenu("Home", "home")
		add_subTopMenu("Reset Stats", "rsstats")
		add_subTopMenu("Setting", "setting")

		add_topMenu("OneProxy", "oneproxy")
		add_subTopMenu("Task", "task")
		add_subTopMenu("TaskThread", "taskthread")
		add_subTopMenu("Lock", "lock")
		add_subTopMenu("HtmlPage", "htmlpage")

		add_topMenu("Statspack", "topsqls")
		add_subTopMenu("Top SQLS", "topsqls")
		add_subTopMenu("Top Clients", "topclients")
		add_subTopMenu("Top Tables", "toptables")
		add_subTopMenu("Top TablesMap", "toptablesmap")
		add_subTopMenu("Top Trans", "toptrans")
		add_subTopMenu("Find Sql", "findsql")

		add_topMenu_realTime("Realtime", "topsqls")
		add_subTopMenu_realTime("Top SQLS", "topsqls")
		add_subTopMenu_realTime("Top Clients", "topclients")
		add_subTopMenu_realTime("Top Tables", "toptables")
		add_subTopMenu_realTime("Top TablesMap", "toptablesmap")
		add_subTopMenu_realTime("Top Trans", "toptrans")
		add_subTopMenu_realTime("Find Sql", "findsql")

		add_topMenu("OneXSoft.com", "http://www.onexsoft.com/zh/oneproxy.html")
		add_subTopMenu("OneSql", "http://www.onexsoft.com/zh/onesql.html")
		add_subTopMenu("OneProxy", "http://www.onexsoft.com/zh/oneproxy.html")
		add_subTopMenu("Blog", "http://www.onexsoft.com/zh/")
		add_subTopMenu("", "")
	};

	StringBuf name, href;
	int i = 0;
	Menu *ptopMenu = NULL;
	for (i = 0; menu[i].menuName.length() > 0; ++i) {
		if (menu[i].is_topMenu) {
			if (menu[i].is_realTime) {
				name.clear();
				href.clear();
				name.appendFormat("%s(%ds)", menu[i].menuName.c_str(), this->httpServerConfig.stats_interval);
				href.appendFormat("%s?realtime=true", menu[i].menuHref.c_str());
				ptopMenu = new Menu(std::string(name.addr(), name.length()), std::string(href.addr(), href.length()));
			} else {
				ptopMenu = new Menu(menu[i].menuName, menu[i].menuHref);
			}
			topMenu.push_back(ptopMenu);
		} else {
			Menu* subMenu = NULL;

			if (menu[i].is_realTime) {
				href.clear();
				href.appendFormat("%s?realtime=true", menu[i].menuHref.c_str());
				subMenu = new Menu(menu[i].menuName, std::string(href.addr(), href.length()));
			} else {
				subMenu = new Menu(menu[i].menuName, menu[i].menuHref);
			}
			if (ptopMenu == NULL) {
				ptopMenu = new Menu("", "");
				topMenu.push_back(ptopMenu);
			}
			ptopMenu->add_subMenu(subMenu);
		}
	}
}

int HttpResponse::response_get(Http& http)
{
	httpHtml.response_setTitle(http);
	httpHtml.response_setMenu(http, this->topMenu);

	this->httpServerConfig.page_need_refresh = true;
	this->httpServerConfig.current_page_home = false;

	FuncMapType::iterator it = this->responseFuncMap.find(http.baseUri);
	if (it != this->responseFuncMap.end()) {
		Func func = it->second;
		(this->*func)(http);

		//record user query data page
		{
			if(record()->httpRequestPageCount.find(http.baseUri) == record()->httpRequestPageCount.end()) {
				record()->httpRequestPageCount[http.baseUri] = 1;
			} else {
				record()->httpRequestPageCount[http.baseUri] += 1;
			}
		}
	} else {
		response_home(http);
	}

	if(this->httpServerConfig.page_need_refresh && this->httpServerConfig.page_refresh_time > 0){
		http.outputBuf.appendFormat("<meta http-equiv=\"refresh\" content=\"%d\">", this->httpServerConfig.page_refresh_time);
	}

	httpHtml.response_endHtmlBody(http);
	http.set_responseStatus(200, "OK");
	return http.send_response();
}

void HttpResponse::response_topclients(Http& http)
{
#define asc(field) order_func(u_uint64, Record::ClientQueryInfoPart, field, true)
#define desc(field) order_func(u_uint64, Record::ClientQueryInfoPart, field, false)
#define order_field(field)  asc(field), desc(field)
#define order_field_bool(field) order_func(bool, Record::ClientQueryInfoPart, field, true), \
	order_func(bool, Record::ClientQueryInfoPart, field, false)

	static TableTitleInfo tti[] = {
			{"HashCode", "hash code", "", "",NULL, NULL},
			{"Ip</br>Address", "client ip address", "", "", NULL, NULL},
			{"Connect", "the number of client connect to oneproxy", "Connect", "", order_field(connectNum)},
			{"Connect</br>ServerFail", "the number of client connect to database failed", "ConnectServerFail", "", order_field(connServerFail)},
			{"SQLs", "the number of SQL that the client execute(include failed sqls) ", "SQLs", "", order_field(sqlSize)},
			{"Query", "queries total", "Query", "", order_field(queryNum)},
			{"Query</br>Failed", "queries failed total", "QueryFailed", "", order_field(queryFailNum)},
			{"Trans", "the number of translation", "Trans", "", order_field(trxNum)},
			{"Select", "select query times", "Select", "", order_field(selectNum)},
			{"Insert", "insert operation times", "Insert", "", order_field(insertNum)},
			{"Update", "update operation times", "Update", "", order_field(updateNum)},
			{"Net</br>Send", "the size of client send to database(KB)", "NetSend", "", order_field(upDataSize)},
			{"Net</br>Recv", "the size of database send to client(KB)", "NetRecv", "", order_field(downDataSize)},
			{"OnLine</br>Time", "the total time of online(s)", "OnLineTime", "", order_field(onLineTime)},
			{"OnLine</br>Status", "current online or not", "OnLineStatus", "", order_field_bool(onLineStatus)},
			{"Latest</br>ConnectTime", "client connect time recently", "LatestConnectTime", "", order_field(start_connect_time)},
			{"", "", "", "", NULL, NULL},
	};
#undef asc
#undef desc
#undef order_field
#undef order_field_bool

	TableTitleInfoVec ttiv;
	ttiv.push_back(tti);

	UriParam uriParam;
	//1. parse uri param
	this->parse_uriParam(http, uriParam);
	Record* record = NULL;
	if (uriParam.isRealTime) {
		record = record()->get_realTimeRecord();
	} else {
		record = record();
	}

	//2. sort
	std::vector<Record::ClientQueryInfo*> clientInfoVec;
	Record::ClientInfoMap tmpMap;
	if (uriParam.sqlhashcode <= 0) {
		this->change_map2vecSort<std::vector<Record::ClientQueryInfo*>, Record::ClientInfoMap, Record::ClientInfoMap::iterator>
		(ttiv, uriParam, clientInfoVec, record->clientQueryMap);
	} else {
		Record::mutex.lock();
		Record::ClientInfoMap::iterator it = record->clientQueryMap.begin();
		for (; it != record->clientQueryMap.end(); ++it) {
			if (it->second.sqlList.find(uriParam.sqlhashcode) != it->second.sqlList.end()) {
				if (it->second.is_show())
					tmpMap[it->first] = it->second;
			}
		}
		Record::mutex.unlock();
		this->change_map2vecSort<std::vector<Record::ClientQueryInfo*>, Record::ClientInfoMap, Record::ClientInfoMap::iterator>
		(ttiv, uriParam, clientInfoVec, tmpMap);
	}

	//3. generate table header
	this->gen_tableTitle(ttiv, uriParam, "Top Clients", clientInfoVec.size(), http);

	//4. generate the table data
	unsigned int startIndex = (uriParam.currentPage - 1) * httpServerConfig.pageSize;
	unsigned int endIndex = (uriParam.currentPage) * httpServerConfig.pageSize;
	if (httpServerConfig.current_page_home && httpServerConfig.home_clients_rows > 0) {
		startIndex = 0;
		endIndex = httpServerConfig.home_clients_rows;
	}
	endIndex = endIndex > clientInfoVec.size() ? clientInfoVec.size() : endIndex;

	for (unsigned int i = startIndex; i < endIndex; ++i) {
		Record::ClientQueryInfo* it = clientInfoVec[i];
		it->part.sqlSize = it->sqlList.size();

		http.outputBuf.append("<tr>");
		http.outputBuf.appendFormat("<td>%u</td>", it->part.hashCode);
		http.outputBuf.appendFormat("<td>%s</td>", it->ipAddr.c_str());
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.connectNum);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.connServerFail);
		http.outputBuf.appendFormat("<td><a href='topsqls?clienthashcode=%u&sqltype=0'>%u</a></td>",it->part.hashCode, it->sqlList.size());
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.queryNum);
		http.outputBuf.appendFormat("<td><a href='topsqls?clienthashcode=%u&sqltype=1'>%lld</a></td>", it->part.hashCode, it->part.queryFailNum);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.trxNum);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.selectNum);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.insertNum);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.updateNum);
		http.outputBuf.appendFormat("<td>%.2f</td>", (it->part.upDataSize * 1.0)/ (1024.0));
		http.outputBuf.appendFormat("<td>%.2f</td>", (it->part.downDataSize * 1.0)/ (1024.0));
		u_uint64 onLineTime = it->part.onLineTime;
		if (it->part.onLineStatus == true) {
			onLineTime += (SystemApi::system_millisecond() - it->part.start_connect_time);
		}
		http.outputBuf.appendFormat("<td>%.2f</td>", (onLineTime * 1.0)/(1000.0));
		if (it->part.onLineStatus == true) {
			http.outputBuf.append("<td>OnLine</td>");
		} else {
			http.outputBuf.append("<td>OffLine</td>");
		}
		http.outputBuf.appendFormat("<td>%s</td>", SystemApi::system_time2Str(((time_t)(it->latest_connect_time))).c_str());
		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</table>");
}

void HttpResponse::response_topsqls(Http& http)
{
	UriParam uriParam;

#define asc(field) order_func(u_uint64, Record::SqlInfoPart, field, true)
#define desc(field) order_func(u_uint64, Record::SqlInfoPart, field, false)
#define order_field(field)  asc(field), desc(field)
	static TableTitleInfo tti[] = {
		init_tableTitle("Hash</br>Code", "the hash value of sql text"),
		init_tableTitle("Sql</br>Text", "the sql to query"),
		init_tableTitle_noattr_order("Tabs", "the number of tables in sql text", "Tabs", order_field(tabs)),
		init_tableTitle_noattr_order("Exec", "the execute times of sql text", "Exec", order_field(exec)),
		init_tableTitle_noattr_order("Trans", "the sql execute times in translation", "Trans", order_field(trans)),
		init_tableTitle_noattr_order("Fails", "the sql execute failed times", "Fails", order_field(fail)),
		init_tableTitle_noattr_order("Execute</br>Time", "the sql execute total time", "Execute", order_field(execTime)),
		init_tableTitle_noattr_order("TotalRow", "the total rows that DB send to client", "TotalRow", order_field(totalRow)),
		init_tableTitle_noattr_order("RecvSize", "the result total size of sql(KB)", "RecvSize", order_field(recvSize)),
		init_tableTitle_noattr_order("ClientNum", "The amount of the client contains the SQL", "ClientNum", order_field(clientSetSize)),
		init_tableTitle_noattr_order("SqlSize", "The length of SQL", "SqlSize", order_field(sqlSize)),
		init_tableTitle_NULL()
	};
#undef asc
#undef desc
#undef order_field

	TableTitleInfoVec ttiv;
	ttiv.push_back(tti);

	//1. parse uri param
	this->parse_uriParam(http, uriParam);
	Record* record = NULL;
	if (uriParam.isRealTime) {
		record = record()->get_realTimeRecord();
	} else {
		record = record();
	}

	//2. sort
	std::vector<Record::SqlInfo*> sqlInfoVec;
	Record::SqlInfoMap tmpMap;
	if (uriParam.clientHashCode > 0) {
		Record::mutex.lock();
		Record::SqlInfoMap::iterator it = record->sqlInfoMap.begin();
		for (; it != record->sqlInfoMap.end(); ++it) {
			if (it->second.clientSet.find(uriParam.clientHashCode) != it->second.clientSet.end()) {
				if (it->second.is_show()) {
					if (uriParam.sqltype == 0) {//show all sql
						tmpMap[it->first] = it->second;
					} else if (it->second.part.fail > 0) {//show fail sql
							tmpMap[it->first] = it->second;
					}
				}
			}
		}
		Record::mutex.unlock();
		this->change_map2vecSort<std::vector<Record::SqlInfo*>, Record::SqlInfoMap, Record::SqlInfoMap::iterator>
		(ttiv, uriParam, sqlInfoVec, tmpMap);
	} else if (uriParam.tableName.size() > 0) {
		Record::mutex.lock();
		Record::SqlInfoMap::iterator it = record->sqlInfoMap.begin();
		for (; it != record->sqlInfoMap.end(); ++it) {
			if (it->second.tableSet.find(uriParam.tableName) != it->second.tableSet.end()) {
				if (it->second.is_show())
					tmpMap[it->first] = it->second;
			}
		}
		Record::mutex.unlock();
		this->change_map2vecSort<std::vector<Record::SqlInfo*>, Record::SqlInfoMap, Record::SqlInfoMap::iterator>
		(ttiv, uriParam, sqlInfoVec, tmpMap);
	} else {
		this->change_map2vecSort<std::vector<Record::SqlInfo*>, Record::SqlInfoMap, Record::SqlInfoMap::iterator>
		(ttiv, uriParam, sqlInfoVec, record->sqlInfoMap);
	}

	//3. generate table header
	this->gen_tableTitle(ttiv, uriParam, "Top SQLS", sqlInfoVec.size(), http);

	//4. generate the table data
	unsigned int startIndex = (uriParam.currentPage - 1) * httpServerConfig.pageSize;
	unsigned int endIndex = (uriParam.currentPage) * httpServerConfig.pageSize;
	if (httpServerConfig.current_page_home && httpServerConfig.home_topsqls_rows > 0) {
		startIndex = 0;
		endIndex = httpServerConfig.home_topsqls_rows;
	}
	endIndex = endIndex > sqlInfoVec.size() ? sqlInfoVec.size() : endIndex;

	for (unsigned int i = startIndex; i < endIndex; ++i) {
		Record::SqlInfo* it = sqlInfoVec[i];
		it->part.clientSetSize = (u_uint64)it->clientSet.size();
		it->part.sqlSize = (u_uint64)it->sqlText.length();
		http.outputBuf.append("<tr>");
		http.outputBuf.appendFormat("<td><a href='findsql?sqlhashcode=%u'>%u</a></td>",
				it->part.hashCode, it->part.hashCode);
		http.outputBuf.appendFormat("<td>%s</td>", Tool::format_string((const char*)it->sqlText.c_str(), 64));
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.tabs);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.exec);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.trans);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.fail);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.execTime);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.totalRow);
		http.outputBuf.appendFormat("<td>%.2f</td>", (it->part.recvSize * 1.0) / 1024.0);
		http.outputBuf.appendFormat("<td><a href='topclients?sqlhashcode=%u'>%u</a></td>",
				it->part.hashCode, it->clientSet.size());
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.sqlSize);

		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</table>");
}

void HttpResponse::response_toptables(Http& http)
{
	UriParam uriParam;

	#define asc(field) order_func(u_uint64, Record::TableInfoPart, field, true)
	#define desc(field) order_func(u_uint64, Record::TableInfoPart, field, false)

	static TableTitleInfo tti1[] = {
			init_tableTitle_nohref("TableName", "the table name in sql", "rowspan='2'"),
			init_tableTitle_all("Sqls", "the total of sql contains the table", "Sqls", "rowspan='2'", asc(sqls), desc(sqls)),
			init_tableTitle_nofunc("Select", "", "", "colspan='3'"),
			init_tableTitle_nofunc("Insert", "", "", "colspan='3'"),
			init_tableTitle_nofunc("Update", "", "", "colspan='3'"),
			init_tableTitle_nofunc("Delete", "", "", "colspan='3'"),
			init_tableTitle_NULL()
		};
	static TableTitleInfo tti2[] = {
			init_tableTitle_noattr("Exec", "the total times of execute query", "SelectExec", asc(selectExec), desc(selectExec)),
			init_tableTitle_noattr("Rows", "the total of return rows", "SelectRows", asc(selectRows), desc(selectRows)),
			init_tableTitle_noattr("Time", "the total execute Time", "SelectTime", asc(selectTime), desc(selectTime)),

			init_tableTitle_noattr("Exec", "the total times of execute Insert", "InsertExec", asc(insertExec), desc(insertExec)),
			init_tableTitle_noattr("Rows", "the total of affect rows", "InsertRows", asc(insertRows), desc(insertRows)),
			init_tableTitle_noattr("Time", "the total execute Time", "InsertTime", asc(insertTime), desc(insertTime)),

			init_tableTitle_noattr("Exec", "the total times of execute update", "UpdateExec", asc(updateExec), desc(updateExec)),
			init_tableTitle_noattr("Rows", "the total of affect rows", "UpdateRows", asc(updateRows), desc(updateRows)),
			init_tableTitle_noattr("Time", "the total execute Time", "UpdateTime", asc(updateTime), desc(updateTime)),

			init_tableTitle_noattr("Exec", "the total times of execute delete", "DeleteExec", asc(deleteExec), desc(deleteExec)),
			init_tableTitle_noattr("Rows", "the total of affect rows", "DeleteRows", asc(deleteRows), desc(deleteRows)),
			init_tableTitle_noattr("Time", "the total execute Time", "DeleteTime", asc(deleteTime), desc(deleteTime)),

			init_tableTitle_NULL()
		};
	#undef asc
	#undef desc

	//1. parse uri param
	this->parse_uriParam(http, uriParam);

	Record* record = NULL;
	if (uriParam.isRealTime) {
		record = record()->get_realTimeRecord();
	} else {
		record = record();
	}

	//get table info base sql map
	Record::TableInfoMap tableInfoMap;
	Record::SqlInfoMap::iterator it = record->sqlInfoMap.begin();
	for (; it != record->sqlInfoMap.end(); ++it) {
		Record::SqlInfo& sqlInfo = it->second;
		if (it->second.is_show() == false)
			continue;
//		sqlInfo.tableSetLock.lock();
		std::set<std::string>::iterator sit = sqlInfo.tableSet.begin();
		for(; sit != sqlInfo.tableSet.end(); ++sit) {
			tableInfoMap[*sit].part.sqls++;
			tableInfoMap[*sit].tableName = *sit;
			switch(sqlInfo.part.type) {
			case stats::sql_op_select:
			{
				tableInfoMap[*sit].part.selectExec += sqlInfo.part.exec;
				tableInfoMap[*sit].part.selectRows += sqlInfo.part.totalRow;
				tableInfoMap[*sit].part.selectTime += sqlInfo.part.execTime;
			}
			break;
			case stats::sql_op_insert:
			{
				tableInfoMap[*sit].part.insertExec += sqlInfo.part.exec;
				tableInfoMap[*sit].part.insertRows += sqlInfo.part.totalRow;
				tableInfoMap[*sit].part.insertTime += sqlInfo.part.execTime;
			}
			break;
			case stats::sql_op_update:
			{
				tableInfoMap[*sit].part.updateExec += sqlInfo.part.exec;
				tableInfoMap[*sit].part.updateRows += sqlInfo.part.totalRow;
				tableInfoMap[*sit].part.updateTime += sqlInfo.part.execTime;
			}
			break;
			case stats::sql_op_delete:
			{
				tableInfoMap[*sit].part.deleteExec += sqlInfo.part.exec;
				tableInfoMap[*sit].part.deleteRows += sqlInfo.part.totalRow;
				tableInfoMap[*sit].part.deleteTime += sqlInfo.part.execTime;
			}
			break;
			default:
				break;

			}
		}
	}

	TableTitleInfoVec ttiv;
	ttiv.push_back(tti1);
	ttiv.push_back(tti2);

	//2. sort
	std::vector<Record::TableInfo*> tableInfoVec;

	this->change_map2vecSort<std::vector<Record::TableInfo*>, Record::TableInfoMap, Record::TableInfoMap::iterator>
	(ttiv, uriParam, tableInfoVec, tableInfoMap);

	//3. generate table header
	this->gen_tableTitle(ttiv, uriParam, "Top Tables", tableInfoVec.size(), http);

	//4. generate the table data
	unsigned int startIndex = (uriParam.currentPage - 1) * httpServerConfig.pageSize;
	unsigned int endIndex = (uriParam.currentPage) * httpServerConfig.pageSize;
	endIndex = endIndex > tableInfoVec.size() ? tableInfoVec.size() : endIndex;
	for (unsigned int i = startIndex; i < endIndex; ++i) {
		Record::TableInfo* it = tableInfoVec[i];
		http.outputBuf.append("<tr>");
		http.outputBuf.appendFormat("<td>%s</td>", it->tableName.c_str());
		http.outputBuf.appendFormat("<td><a href='topsqls?tablename=%s'>%lld</a></td>",
				it->tableName.c_str(), it->part.sqls);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.selectExec);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.selectRows);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.selectTime);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.insertExec);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.insertRows);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.insertTime);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.updateExec);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.updateRows);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.updateTime);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.deleteExec);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.deleteRows);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.deleteTime);
		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</table>");
}

void HttpResponse::response_toptablesMap(Http& http)
{
	UriParam uriParam;

	#define asc(field) order_func(u_uint64, Record::TableMapInfoPart, field, true)
	#define desc(field) order_func(u_uint64, Record::TableMapInfoPart, field, false)
	#define order_field(field)  asc(field), desc(field)
	static TableTitleInfo tti[] = {
			{"Tables", "the name of tables in sql, the start of qry represent simple query; trx represent in translation", "", "", NULL, NULL},
			{"Exec", "the times of execute", "exec", "", order_field(exec)},
			{"SqlHashCode", "the sql hash code that contains the tables", "", "", NULL, NULL},
			{"", "", "", "", NULL, NULL}
		};
	#undef asc
	#undef desc
	#undef order_field

	//1. parse uri param
	this->parse_uriParam(http, uriParam);

	Record::TableMapInfoMap tableMapInfoMap;
	{
		Record* record = NULL;
		if (uriParam.isRealTime) {
			record = record()->get_realTimeRecord();
		} else {
			record = record();
		}

		//从sqlInfoMap中抽取信息
		StringBuf tmpBuf;
		Record::SqlInfoMap& sqlInfoMap = record->sqlInfoMap;
		Record::SqlInfoMap::iterator it = sqlInfoMap.begin();
		for (; it != sqlInfoMap.end(); ++it) {

			if (it->second.is_show() == false)
				continue;

			if (it->second.tableSet.size() <= 0) {
				continue;
			}

			tmpBuf.clear();
			if (it->second.part.trans > 0) {
				tmpBuf.append("trx:");
			} else {
				tmpBuf.append("qry:");
			}

//			it->second.tableSetLock.lock();
			std::set<std::string>::iterator sit = it->second.tableSet.begin();
			for (; sit != it->second.tableSet.end(); ++sit) {
				if (sit == it->second.tableSet.begin()){
					tmpBuf.append((*sit).c_str());
				} else {
					tmpBuf.appendFormat(",%s", (*sit).c_str());
				}
			}
//			it->second.tableSetLock.unlock();

			std::string key(tmpBuf.addr(), tmpBuf.length());
			tableMapInfoMap[key].tablesName = key;
			tableMapInfoMap[key].part.exec += it->second.part.exec;
			tableMapInfoMap[key].sqlHashCode.insert(it->first);
		}
	}

	TableTitleInfoVec ttiv;
	ttiv.push_back(tti);

	//2. sort
	std::vector<Record::TableMapInfo*> tableMapInfoVec;

	this->change_map2vecSort<std::vector<Record::TableMapInfo*>, Record::TableMapInfoMap, Record::TableMapInfoMap::iterator>
	(ttiv, uriParam, tableMapInfoVec, tableMapInfoMap);

	//3. generate table header
	this->gen_tableTitle(ttiv, uriParam, "Top TablesMap", tableMapInfoVec.size(), http);

	//4. generate the table data
	StringBuf tmpBuf;
	unsigned int startIndex = (uriParam.currentPage - 1) * httpServerConfig.pageSize;
	unsigned int endIndex = (uriParam.currentPage) * httpServerConfig.pageSize;
	endIndex = endIndex > tableMapInfoVec.size() ? tableMapInfoVec.size() : endIndex;
	for (unsigned int i = startIndex; i < endIndex; ++i) {
		Record::TableMapInfo* it = tableMapInfoVec[i];
		http.outputBuf.append("<tr>");
		http.outputBuf.appendFormat("<td>%s</td>", it->tablesName.c_str());
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.exec);

		tmpBuf.clear();
		std::set<unsigned int>::iterator sit = it->sqlHashCode.begin();
		for (; sit != it->sqlHashCode.end(); ++sit) {
			if (tmpBuf.length() > 0) {
				tmpBuf.append(",");
			}
			tmpBuf.appendFormat("<a href='findsql?sqlhashcode=%u'>%u</a>", (*sit), (*sit));
		}
		tmpBuf.appendFormat("%c", '\0');

		http.outputBuf.appendFormat("<td>%s</td>", tmpBuf.addr());
		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</table>");
}

void HttpResponse::response_topTrans(Http& http)
{
	UriParam uriParam;

#define asc(field) order_func(u_uint64, Record::TransInfoPart, field, true)
#define desc(field) order_func(u_uint64, Record::TransInfoPart, field, false)
#define order_field(field)  asc(field), desc(field)
	static TableTitleInfo tti[] = {
		init_tableTitle("Trans</br>HashCode", "the hash code of translation"),
		init_tableTitle("Sql</br>List", "the hash code of sql list"),
		init_tableTitle("Sql</br>Text", "the sql text of sql list"),
		init_tableTitle_noattr_order("Exec", "the execute times of sql text", "Exec", order_field(exec)),
		init_tableTitle_noattr_order("TotalTime", "total execute time of all sql(s)", "TotalTime", order_field(totalTime)),
		init_tableTitle_noattr_order("RollBack</br>Times", "the amount of roll back sqls", "Fails", order_field(rollbackTimes)),
		init_tableTitle_noattr_order("SqlNum", "the amount of sql in translation", "SqlNum", order_field(sqlNum)),
		init_tableTitle_noattr_order("MaxTime", "the max time of execute translation(ms)", "MaxTime", order_field(maxTime)),
		init_tableTitle_noattr_order("MinTime", "the min time of execute translation(ms)", "MinTime", order_field(minTime)),
		init_tableTitle_noattr_order("LastTime", "The last time of translation(ms)", "LastTime", order_field(lastTime)),
		init_tableTitle("Last<br>StartTime", "The start time of the last translation"),
		init_tableTitle_NULL()
	};
#undef asc
#undef desc
#undef order_field

	TableTitleInfoVec ttiv;
	ttiv.push_back(tti);

	//1. parse uri param
	this->parse_uriParam(http, uriParam);
	Record* record = NULL;
	if (uriParam.isRealTime) {
		record = record()->get_realTimeRecord();
	} else {
		record = record();
	}


	//2. sort
	std::vector<Record::TransInfo*> transInfoVec;
	this->change_map2vecSort<std::vector<Record::TransInfo*>, Record::TransInfoMap, Record::TransInfoMap::iterator>
	(ttiv, uriParam, transInfoVec, record->transInfoMap);


	//3. generate table header
	this->gen_tableTitle(ttiv, uriParam, "Top Trans", transInfoVec.size(), http);

	//4. generate the table data
	unsigned int startIndex = (uriParam.currentPage - 1) * httpServerConfig.pageSize;
	unsigned int endIndex = (uriParam.currentPage) * httpServerConfig.pageSize;
	if (httpServerConfig.current_page_home && httpServerConfig.home_topsqls_rows > 0) {
		startIndex = 0;
		endIndex = httpServerConfig.home_topsqls_rows;
	}
	endIndex = endIndex > transInfoVec.size() ? transInfoVec.size() : endIndex;

	for (unsigned int i = startIndex; i < endIndex; ++i) {
		Record::TransInfo* it = transInfoVec[i];

		http.outputBuf.append("<tr>");

		StringBuf sqlHashCode, sqlText;
		std::set<unsigned int>::iterator its = it->sqlHashCode.begin();
		for (; its != it->sqlHashCode.end(); ++its) {
			sqlHashCode.appendFormat("<a href='findsql?sqlhashcode=%u'>%u;</a>", *its, *its);
			Record::SqlInfo* si = record->find_sqlInfo(*its);
			if (si != NULL) {
				sqlText.appendFormat("%s;", si->sqlText.c_str());
			}
		}

		http.outputBuf.appendFormat("<td><a href='findsql?transhashcode=%u'>%u</a></td>",
				it->part.transHashCode, it->part.transHashCode);
		http.outputBuf.appendFormat("<td>%s</td>", sqlHashCode.addr());
		http.outputBuf.appendFormat("<td>%s</td>", Tool::format_string(sqlText.addr(), 64));
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.exec);
		http.outputBuf.appendFormat("<td>%.2f</td>", (it->part.totalTime * 1.0)/1000.0);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.rollbackTimes);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.sqlNum);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.maxTime);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.minTime);
		http.outputBuf.appendFormat("<td>%lld</td>", it->part.lastTime);
		http.outputBuf.appendFormat("<td>%s</td>", SystemApi::system_time2Str((time_t)it->lastTime/1000).c_str());

		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</table>");
}

void HttpResponse::response_findsql(Http& http)
{
	UriParam uriParam;
	this->parse_uriParam(http, uriParam);
	this->httpServerConfig.page_need_refresh = false;

	http.outputBuf.append("<table border='0' class='data' width='98%%'>");
	http.outputBuf.append("<caption>");
	http.outputBuf.append("<form action='findsql' method='get'>");
	http.outputBuf.appendFormat("Hashcode: <input class='inputs' type='text' name='sql' value='%u'/>", uriParam.sqlhashcode);
	http.outputBuf.append("<input class='buttons' type='submit' value='Find' />");
	http.outputBuf.append("</form>");
	http.outputBuf.append("</caption>");

	http.outputBuf.append("<tr>");
	http.outputBuf.append("<th>Field</td>");
	http.outputBuf.append("<th>Value</td>");
	http.outputBuf.append("</tr>");

	Record* record = NULL;
	if (uriParam.isRealTime) {
		record = record()->get_realTimeRecord();
	} else {
		record = record();
	}

	Record::TransInfoMap::iterator tit = record->transInfoMap.find(uriParam.transhashcode);
	Record::SqlInfoMap::iterator it = record->sqlInfoMap.find(uriParam.sqlhashcode);
	if (uriParam.sqlhashcode != 0 && it != record->sqlInfoMap.end())
	{
		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>Hashcode</td>");
		http.outputBuf.appendFormat("<td>%u</td>", it->second.part.hashCode);
		http.outputBuf.append("</tr>");

		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>SQL Text</td>");
		http.outputBuf.appendFormat("<td>%s</td>", it->second.sqlText.c_str());
		http.outputBuf.append("</tr>");

		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>Exec</td>");
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.part.exec);
		http.outputBuf.append("</tr>");


		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>Total Time</td>");
		http.outputBuf.appendFormat("<td>%.2f</td>", (it->second.part.execTime*1.0)/1000.0);
		http.outputBuf.append("</tr>");

		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>Rows</td>");
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.part.totalRow);
		http.outputBuf.append("</tr>");

		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>TableNum</td>");
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.part.tabs);
		http.outputBuf.append("</tr>");

		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>ClientNum</td>");
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.clientSet.size());
		http.outputBuf.append("</tr>");
	} else if (uriParam.transhashcode != 0 && tit != record->transInfoMap.end()){

		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>Hashcode</td>");
		http.outputBuf.appendFormat("<td>%u</td>", tit->second.part.transHashCode);
		http.outputBuf.append("</tr>");

		StringBuf sqlHashCode, sqlText;
		std::set<unsigned int>::iterator sit =  tit->second.sqlHashCode.begin();
		for (; sit != tit->second.sqlHashCode.end(); ++sit) {
			sqlHashCode.appendFormat("<a href='findsql?sqlhashcode=%u'>%u;</a>", *sit, *sit);
			Record::SqlInfo* si = record->find_sqlInfo(*sit);
			if (si != NULL) {
				sqlText.appendFormat("%s;", si->sqlText.c_str());
			}
		}

		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>SQL HashCodes</td>");
		http.outputBuf.appendFormat("<td>%s</td>", sqlHashCode.addr());
		http.outputBuf.append("</tr>");


		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>SQL Texts</td>");
		http.outputBuf.appendFormat("<td>%s</td>", sqlText.addr());
		http.outputBuf.append("</tr>");
	} else {
		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>Hashcode</td>");
		http.outputBuf.appendFormat("<td>%u</td>", uriParam.sqlhashcode);
		http.outputBuf.append("</tr>");

		http.outputBuf.append("<tr>");
		http.outputBuf.append("<td width='10%%'>SQL Text</td>");
		http.outputBuf.append("<td></td>");
		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</table>");
}

void HttpResponse::response_home(Http& http)
{
	this->httpServerConfig.current_page_home = true;

	http.outputBuf.appendFormat("<h3>welcome use oneproxy statistics system</br>(%s)</h3>",
			SystemApi::system_timeStr().c_str());

#ifdef linux
	this->response_machine(http);
#endif

	if (httpServerConfig.show_topsqls) {
		http.outputBuf.appendFormat("<span align='center'>(Sql List, Top by Exec)</span>");
		http.set_uriParam("fd", "Exec");
		this->response_topsqls(http);
	}

	if (httpServerConfig.show_clients) {
		http.outputBuf.appendFormat("<span align='center'>(Client List, Top by Query)</span>");
		http.set_uriParam("fd", "Query");
		this->response_topclients(http);
	}

	if (httpServerConfig.show_taskThread) {
		http.outputBuf.appendFormat("<span align='center'>(TaskThread List)</span>");
		this->response_taskThread(http);
	}
}

void HttpResponse::response_machine(Http& http)
{
	OSLoadRecord osr;
	CurrentProcessInfo cpi;

	this->machineStatus.get_machineStatus(osr);
	this->machineStatus.get_systemCurrentProcessInfo(cpi);

	http.outputBuf.appendFormat("<table border='0' class='data' width='98%%'>");

	if (this->httpServerConfig.current_page_home == false) {
		http.outputBuf.appendFormat("<caption>OneProxy Machine Statistics</caption>");
	}

	http.outputBuf.appendFormat("<tr>");
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Sys</th><td align='left' width='10%%'>%u%%</td>", osr.system);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Usr</th><td align='left' width='10%%'>%u%%</td>", osr.user);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Idle</th><td align='left' width='10%%'>%u%%</td>", 100 - osr.system - osr.iowait - osr.softirq - osr.user);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Wio</th><td align='left' width='10%%'>%u%%</td>", osr.iowait);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Irq</th><td align='left' width='10%%'>%u%%</td>", osr.softirq);
	http.outputBuf.appendFormat("</tr>");

	http.outputBuf.appendFormat("<tr>");
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Load</th><td align='left' width='10%%'>%.2f</td>", osr.load);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Uptime</th><td align='left' width='10%%'>%u</td>", osr.uptime);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Run</th><td align='left' width='10%%'>%u</td>", osr.run);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Procs</th><td align='left' width='10%%'>%u</td>", osr.num);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Fork</th><td align='left' width='10%%'>%u</td>", osr.fork);
	http.outputBuf.appendFormat("</tr>");

	http.outputBuf.appendFormat("<tr>");
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Swap</th><td align='left' width='10%%'>%.3f GB</td>", osr.swap/1024/1024/1024);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Swap In</th><td align='left' width='10%%'>%u</td>", osr.swpin);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Swap Out</th><td align='left' width='10%%'>%u</td>", osr.swpout);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Free</th><td align='left' width='10%%'>%.3f GB</td>", osr.free/1024/1024/1024);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Cache</th><td align='left' width='10%%'>%.3f GB</td>", osr.cache/1024/1024/1024);
	http.outputBuf.appendFormat("</tr>");

	http.outputBuf.appendFormat("<tr>");
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Net In</th><td align='left' width='10%%'>%.3f MB</td>", osr.ibytes/1048576.0);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Net Out</th><td align='left' width='10%%'>%.3f MB</td>", osr.obytes/1048576.0);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Net Err</th><td align='left' width='10%%'>%u</td>", osr.ierrors + osr.oerrors);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>FileOpen</th><td align='left' width='10%%'>%u</td>", osr.nropen);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>FileTotal</th><td align='left' width='10%%'>%u</td>", osr.nrtotal);
	http.outputBuf.appendFormat("</tr>");

	http.outputBuf.appendFormat("<tr>");
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Tcp Use</th><td align='left' width='10%%'>%u</td>", osr.tcpuse);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Tcp Alloc</th><td align='left' width='10%%'>%u</td>", osr.tcpalloc);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Tcp Wait</th><td align='left' width='10%%'>%u</td>", osr.tcpwait);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Connect</th><td align='left' width='10%%'>%u</td>", osr.aopen);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Accept</th><td align='left' width='10%%'>%u</td>", osr.popen);
	http.outputBuf.appendFormat("</tr>");

	http.outputBuf.appendFormat("<tr>");
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Oneproxy VIRT</th><td align='left' width='10%%'>%s</td>", cpi.virt);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Oneproxy RES</th><td align='left' width='10%%'>%s</td>", cpi.res);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Oneproxy SHR</th><td align='left' width='10%%'>%s</td>", cpi.shr);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Oneproxy CPU</th><td align='left' width='10%%'>%.1f%%</td>", cpi.cpu);
	http.outputBuf.appendFormat("<th align='center' width='10%%'>Oneproxy MEM</th><td align='left' width='10%%'>%.1f%%</td>", cpi.mem);
	http.outputBuf.appendFormat("</tr>");

	http.outputBuf.appendFormat("</table>");
}

void HttpResponse::response_setting(Http& http)
{
	httpServerConfig.page_need_refresh = false;

#define html http.outputBuf.appendFormat
	html("<form action=\"savesetting\" method=\"get\">");
	html("<table border='0' class='data' width='50%%'>");
	html("<caption><h2>Settings</h2></caption");
	html("<tr>");
	html("<th width='20%%' align='center'>name</th>");
	html("<th width='10%%' align='center'>value</th>");
	html("</tr>");

	html("<tr>");
	html("<th>Page Refresh Time</th>");
	html("<th align=\"left\"><input type=\"text\" name=\"page_refresh_time\" size=\"30\" value=\"%d\"></th>", httpServerConfig.page_refresh_time);
	html("</tr>");

	html("<tr>");
	html("<th>Home &nbsp;Sqls&nbsp; Rows</th>");
	html("<th align=\"left\"><input type=\"text\" name=\"home_sqls_rows\" size=\"30\" value=\"%d\"></th>", httpServerConfig.home_topsqls_rows);
	html("</tr>");

	html("<tr>");
	html("<th>Home Client Rows</th>");
	html("<th align=\"left\"><input type=\"text\" name=\"home_client_rows\" size=\"30\" value=\"%d\"></th>", httpServerConfig.home_clients_rows);
	html("</tr>");

	html("<tr>");
	html("<th>Home TaskThread Rows</th>");
	html("<th align=\"left\"><input type=\"text\" name=\"home_taskthread_rows\" size=\"30\" value=\"%d\"></th>", httpServerConfig.home_taskThread_rows);
	html("</tr>");

	html("<tr>");
	html("<th>Home &nbsp;Show &nbsp;Item</th>");
	html("<th align=\"left\"><label><input type=\"checkbox\" name=\"show_sqls\" value=\"1\" %s />Top SQLS</label><br/>"
			"<label><input type=\"checkbox\" name=\"show_client\" value=\"1\" %s />Top Clients</label><br/>"
			"<label><input type=\"checkbox\" name=\"show_taskThread\" value=\"1\" %s />Task Thread</label><br/>",
			httpServerConfig.show_topsqls?"checked=\"true\"":"", httpServerConfig.show_clients?"checked=\"true\"":"",
					httpServerConfig.show_taskThread?"checked=\"true\"":"");
	html("</tr>");

	html("<tr>");
	html("<th>Page&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Size</th>");
	html("<th align=\"left\"><input type=\"text\" name=\"page_size\" size=\"30\" value=\"%d\"></th>", httpServerConfig.pageSize);
	html("</tr>");

	html("<tr>");
	html("<th>Stats&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Interval</th>");
	html("<th align=\"left\"><input type=\"text\" name=\"stats_interval\" size=\"30\" value=\"%d\"></th>", httpServerConfig.stats_interval);
	html("</tr>");

	html("<tr>");
	html("<th colspan=\"2\" align=\"center\"><input type=\"submit\" value=\"Submit\"></th>");
	html("</tr>");
	html("</table>");
	html("</form>");
#undef html
}

void HttpResponse::response_saveSetting(Http& http)
{
	this->httpServerConfig.page_need_refresh = false;
	this->parse_httpServerConfig(http);
	if(this->httpServerConfig.is_autoReset){
		http.outputBuf.appendFormat("<meta http-equiv=\"refresh\" content=\"0;url=home\">");
	} else {
		http.outputBuf.append("save setting success");
		http.outputBuf.appendFormat("<meta http-equiv=\"refresh\" content=\"%d;url=home\">",
				this->httpServerConfig.reset_page_show_time);
	}
	this->clear_topMenu();
	this->response_setMenu(this->topMenu);
}

void HttpResponse::response_reset(Http& http)
{
	this->httpServerConfig.page_need_refresh = false;
	record()->record_clear();

	if(this->httpServerConfig.is_autoReset){
		http.outputBuf.appendFormat("<meta http-equiv=\"refresh\" content=\"0;url=home\">");
	} else {
		http.outputBuf.append("reset success");
		http.outputBuf.appendFormat("<meta http-equiv=\"refresh\" content=\"%d;url=home\">",
				this->httpServerConfig.reset_page_show_time);
	}
}

void HttpResponse::response_task(Http& http)
{
	http.outputBuf.append("<table border='0' class='data' width='98%%'>");
	http.outputBuf.append("<caption>task Information</caption>");
	http.outputBuf.append("<tr>");
	http.outputBuf.append("<th>all task</th>");
	http.outputBuf.append("<th>current task</th>");
	http.outputBuf.append("<th>doing task</th>");
	http.outputBuf.append("<th>wait task</th>");
	http.outputBuf.append("<th>close task</th>");
	http.outputBuf.append("</tr>");
	http.outputBuf.append("<tr>");
	http.outputBuf.appendFormat("<td>%lld</td>", record()->sum_clientConn);
	http.outputBuf.appendFormat("<td>%lld</td>", record()->sum_currentClientConn);
	http.outputBuf.appendFormat("<td>%lld</td>", record()->sum_handingClientConn);
	http.outputBuf.appendFormat("<td>%lld</td>", record()->sum_waitClientConn);
	http.outputBuf.appendFormat("<td>%lld</td>", record()->sum_closeClientConn);
	http.outputBuf.append("</tr>");
	http.outputBuf.append("</table>");
}

void HttpResponse::response_taskThread(Http& http)
{
	http.outputBuf.append("<table border='0' class='data' width='98%%'>");
	if (this->httpServerConfig.current_page_home == false)
		http.outputBuf.append("<caption>task thread Information</caption>");
	http.outputBuf.append("<tr>");
	http.outputBuf.append("<th>thread id</th>");
	http.outputBuf.append("<th>thread all task</th>");
	http.outputBuf.append("<th>thread doing task</th>");
	http.outputBuf.append("<th>thread fail task</th>");
	http.outputBuf.append("<th>thread finished task</th>");
	http.outputBuf.append("</tr>");

	Record::ThreadInfoMap::iterator it = record()->threadInfoMap.begin();
	for (; it != record()->threadInfoMap.end(); ++it) {
		if (it->second.is_show() == false)
			continue;
		http.outputBuf.append("<tr>");
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.thread_id);
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.sum_allClientConn);
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.sum_handingClientConn);
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.sum_failClientConn);
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.sum_finishedClientConn);
		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</table>");
}

void HttpResponse::response_lock(Http& http)
{
	http.outputBuf.append("<table border='0' class='data' width='98%%'>");
	http.outputBuf.append("<caption>Lock Information</caption>");
	http.outputBuf.append("<tr>");
	http.outputBuf.append("<th>lock name</th>");
	http.outputBuf.append("<th>lock num</th>");
	http.outputBuf.append("<th>unlock num</th>");
	http.outputBuf.append("</tr>");

	Record::RecordMutexMap::iterator it =  record()->recordMutexMap.begin();
	for (; it != record()->recordMutexMap.end(); ++it) {
		if (it->second.is_show() == false)
			continue;

		http.outputBuf.append("<tr>");
		http.outputBuf.appendFormat("<td>%s</td>", it->first.c_str());
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.lockNum);
		http.outputBuf.appendFormat("<td>%lld</td>", it->second.unlockNum);
		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</table>");
}

void HttpResponse::response_htmlPage(Http& http)
{
	http.outputBuf.append("<table border='0' class='data' width='98%%'>");
	http.outputBuf.append("<caption>Html Page</caption>");
	http.outputBuf.append("<tr>");
	http.outputBuf.append("<th>Base Url</th>");
	http.outputBuf.append("<th>Request Num</th>");
	http.outputBuf.append("</tr>");

	Record::RequestPageCountMap::iterator it =  record()->httpRequestPageCount.begin();
	for (; it != record()->httpRequestPageCount.end(); ++it) {
		http.outputBuf.append("<tr>");
		http.outputBuf.appendFormat("<td>%s</td>", it->first.c_str());
		http.outputBuf.appendFormat("<td>%lld</td>", it->second);
		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</table>");
}

void HttpResponse::parse_uriParam(Http& http, UriParam& uriParam)
{
	std::string orderBy = http.get_uriParam("orderby");
	std::string page = http.get_uriParam("page");
	uriParam.orderField = http.get_uriParam("fd");
	std::string clientHashCode = http.get_uriParam("clienthashcode");
	std::string sqlHashCode = http.get_uriParam("sqlhashcode");
	uriParam.tableName = http.get_uriParam("tablename");
	std::string sqlType = http.get_uriParam("sqltype");
	std::string transhashcode = http.get_uriParam("transhashcode");
	std::string realTime = http.get_uriParam("realtime");

	if (orderBy.compare("1") == 0) {
		uriParam.orderBy = 1;
	} else if (orderBy.compare("2") == 0) {
		uriParam.orderBy = 2;
	} else {
		uriParam.orderBy = 1;
	}

	if (realTime.size() > 0) {
		uriParam.isRealTime = true;
	} else {
		uriParam.isRealTime = false;
	}

	uriParam.currentPage = atoi(page.c_str());
	if (uriParam.currentPage <= 0)
		uriParam.currentPage = 1;

	uriParam.clientHashCode = (unsigned int)strtoul(clientHashCode.c_str(), NULL, 10);
	uriParam.sqlhashcode = (unsigned int)strtoul(sqlHashCode.c_str(), NULL, 10);
	uriParam.transhashcode = (unsigned int)strtoul(transhashcode.c_str(), NULL, 10);

	if (sqlType.size() < 0) {
		uriParam.sqltype = -1;
	}

	uriParam.sqltype = atoi(sqlType.c_str());
	if (uriParam.sqltype != 0 && uriParam.sqltype != 1) {
		uriParam.sqltype = -1;
	}
}

void HttpResponse::parse_httpServerConfig(Http& http)
{
	//home_refresh_time=5&page_size
	std::string hrt = http.get_uriParam("page_refresh_time");
	std::string ps = http.get_uriParam("page_size");
	std::string hsr = http.get_uriParam("home_sqls_rows");
	std::string hcr = http.get_uriParam("home_client_rows");
	std::string htr = http.get_uriParam("home_taskthread_rows");

	int ihrt = atoi(hrt.c_str());
	if (ihrt > 0)
		httpServerConfig.page_refresh_time = ihrt;

	int ips = atoi(ps.c_str());
	if (ips > 0)
		httpServerConfig.pageSize = ips;

	int ihsr = atoi(hsr.c_str());
	if (ihsr > 0) {
		httpServerConfig.home_topsqls_rows = ihsr;
	}

	int ihcr = atoi(hcr.c_str());
	if (ihcr > 0) {
		httpServerConfig.home_clients_rows = ihcr;
	}

	int ihtr = atoi(htr.c_str());
	if (ihtr > 0) {
		httpServerConfig.home_taskThread_rows = ihtr;
	}

	std::string showsqls = http.get_uriParam("show_sqls");
	std::string showclient = http.get_uriParam("show_client");
	std::string showtaskthread = http.get_uriParam("show_taskThread");
	if (showsqls.size() > 0 || showclient.size() > 0 || showtaskthread.size() > 0) {
		if (showsqls.size() > 0)
			httpServerConfig.show_topsqls = true;
		else
			httpServerConfig.show_topsqls = false;

		if (showclient.size() > 0) {
			httpServerConfig.show_clients = true;
		} else {
			httpServerConfig.show_clients = false;
		}

		if (showtaskthread.size() > 0) {
			httpServerConfig.show_taskThread = true;
		} else {
			httpServerConfig.show_taskThread = false;
		}
	}

	std::string realTime = http.get_uriParam("stats_interval");
	if (realTime.size() > 0) {
		httpServerConfig.stats_interval = atoi(realTime.c_str());
		if (httpServerConfig.stats_interval <= 0) {
			httpServerConfig.stats_interval = 10;
		}
		record()->realRecordTime = httpServerConfig.stats_interval;
	}
}

void HttpResponse::gen_tableTitle(const TableTitleInfoVec& titleInfo, UriParam& uriParam, const char* caption, const int totalItem, Http& http)
{
	int orderby = uriParam.orderBy;
	if (orderby < 0 || orderby > 2)
		orderby = 0;

	if (uriParam.currentPage <= 1) {
		if (uriParam.orderBy == 1)
			orderby = 2;
		else
			orderby = 1;
	}

	std::string baseUri;
	std::stringstream ss;
	ss << http.baseUri.c_str() << "?";
	baseUri = ss.str();

	if (uriParam.isRealTime) {
		ss << "realtime=true" << "&";
		baseUri = ss.str();
	}

	if (uriParam.clientHashCode > 0) {
		ss << "clienthashcode=" << uriParam.clientHashCode<<"&";
		baseUri = ss.str();
	}
	if (uriParam.sqlhashcode > 0) {
		ss << "sqlhashcode=" << uriParam.sqlhashcode << "&";
		baseUri == ss.str();
	}
	if (uriParam.tableName.size() > 0) {
		ss << "tablename=" << uriParam.tableName.c_str() << "&";
		baseUri = ss.str();
	}

	http.outputBuf.append("<table border='0' class='data' width='98%%'>");
	if (caption != NULL) {
		http.outputBuf.append("<caption>");

		if (uriParam.currentPage > 1) {
			http.outputBuf.appendFormat("<a href='%sfd=%s&orderby=%d&page=%d'>Previous</a>",
					baseUri.c_str(), uriParam.orderField.c_str(), uriParam.orderBy, uriParam.currentPage - 1);
		}

		if (this->httpServerConfig.current_page_home == false) {
			http.outputBuf.append("(");
			http.outputBuf.appendFormat("Since %s", SystemApi::system_timeStr().c_str());
			http.outputBuf.appendFormat(" %s,", caption);
			if (uriParam.orderField.length() > 0)
				http.outputBuf.appendFormat(" Order By %s,", uriParam.orderField.c_str());
			if (totalItem > 0) {
				http.outputBuf.appendFormat(" Total: %d rows", totalItem);
			}
			http.outputBuf.append(")");

			//calc sum page
			unsigned int tpage = totalItem / httpServerConfig.pageSize;
			unsigned int totalPage = totalItem % httpServerConfig.pageSize == 0 ? tpage : tpage + 1;
			if (totalPage > uriParam.currentPage)
				http.outputBuf.appendFormat("<a href='%sfd=%s&orderby=%d&page=%d'>Next</a>",
						baseUri.c_str(), uriParam.orderField.c_str(), uriParam.orderBy, uriParam.currentPage + 1);
		}
		http.outputBuf.append("</caption>");

	}
	http.outputBuf.append("<head>");

	for (unsigned int k = 0; k < titleInfo.size(); ++k)
	{
		TableTitleInfo* tti = (TableTitleInfo*)titleInfo[k];
		http.outputBuf.append("<tr align='center'>");

		for (int i = 0; tti[i].name.size() > 0; ++i) {
			if (tti[i].tdattr.length() > 0) {
				http.outputBuf.appendFormat("<th %s >", tti[i].tdattr.c_str());
			} else {
				http.outputBuf.append("<th>");
			}
			if (tti[i].ascFunc == NULL) {
				http.outputBuf.appendFormat("<span title='%s'>%s</span>",tti[i].hint.c_str(), tti[i].name.c_str());
			} else {
				if ((uriParam.orderField.size() > 0) && (tti[i].href.compare(uriParam.orderField) == 0)) {
					http.outputBuf.appendFormat("<a title='%s' href='%sfd=%s&orderby=%d&page=%d'>%s</a>",
							tti[i].hint.c_str(),
							baseUri.c_str(), tti[i].href.c_str(), orderby, uriParam.currentPage,
							tti[i].name.c_str());
				} else {
					http.outputBuf.appendFormat("<a title='%s' href='%sfd=%s&orderby=2&page=1'>%s</a>",
							tti[i].hint.c_str(),
							baseUri.c_str(), tti[i].href.c_str(),
							tti[i].name.c_str());
				}
			}
			http.outputBuf.append("</th>");
		}
		http.outputBuf.append("</tr>");
	}
	http.outputBuf.append("</head>");
}

template<class S, int offset, bool asc>
bool HttpResponse::compare(void* p1, void* p2)
{
	S* a = (S*)(((char*)p1) + offset);
	S* b = (S*)(((char*)p2) + offset);

	if (asc) {
		return (*a) < (*b);
	} else {
		return (*a) > (*b);
	}
}

template<typename VecType, typename MapType, class mapIterType>
void HttpResponse::change_map2vecSort(const TableTitleInfoVec& ttiVec, const UriParam& uriParam, VecType& vec, MapType& mapobject)
{
	mapIterType it = mapobject.begin();
	for (; it != mapobject.end(); ++it) {
		if (it->second.is_show() == false)
			continue;
		vec.push_back(&it->second);
	}

	if ((uriParam.orderBy == 1 || uriParam.orderBy == 2) && uriParam.orderField.size() > 0) {
		for(unsigned int k = 0; k < ttiVec.size(); ++k){
			TableTitleInfo* tti = (TableTitleInfo*)ttiVec[k];
			for (int i = 0; tti[i].name.size() > 0; ++i) {
				if (tti[i].ascFunc != NULL && tti[i].href.compare(uriParam.orderField) == 0) {
					if (uriParam.orderBy == 1) {
						sort(vec.begin(), vec.end(), tti[i].ascFunc);
					} else if (uriParam.orderBy == 2) {
						sort(vec.begin(), vec.end(), tti[i].descFunc);
					}
				}
			}
		}
	}
}

void HttpResponse::clear_topMenu()
{
	TopMenu::iterator it = topMenu.begin();
	for(; it != topMenu.end();) {
		Menu* tTopMenu = *it;
		TopMenu::iterator subit = tTopMenu->get_subMenu().begin();
		for (; subit != tTopMenu->get_subMenu().end();) {
			Menu* sub = *subit;
			++subit;
			delete sub;
		}
		++it;
		delete tTopMenu;
	}
	topMenu.clear();
}
