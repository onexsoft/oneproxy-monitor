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
* @FileName: httpresponse.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月15日
*
*/

#ifndef HTTPSERVER_HTTPRESPONSE_H_
#define HTTPSERVER_HTTPRESPONSE_H_

#include <iostream>
#include <string>
#include <map>

#include "http.h"
#include "httphtml.h"
#include "define.h"
#include "machinestatus.h"

typedef struct _menu_t{
	bool is_topMenu;
	std::string menuName;
	std::string menuHref;
	bool is_realTime;
}Menu_t;
#define add_topMenu(menuName, menuHref) {true, menuName, menuHref, false},
#define add_topMenu_realTime(menuName, menuHref) {true, menuName, menuHref, true},
#define add_subTopMenu(menuName, menuHref) {false, menuName, menuHref, false},
#define add_subTopMenu_realTime(menuName, menuHref) {false, menuName, menuHref, true},

typedef struct _uri_param_t{
	std::string orderField;
	unsigned int orderBy; // 0: rand order, 1: asc order 2: desc order
	unsigned int currentPage; //current page
	unsigned int clientHashCode;//when show sqls uses
	unsigned int sqlhashcode; //when show clients used
	std::string tableName; //when show sqls used.
	int sqltype; //0: query sql list, 1: query fail sql list
	unsigned int transhashcode; //when find trans used.
	unsigned int clientuserappinfohashcode; //when top who's sql used.
	//real stat time
	bool isRealTime;
	_uri_param_t() {
		this->orderBy = 0;
		this->currentPage = 0;
		this->clientHashCode = 0;
		this->sqlhashcode = 0;
		this->sqltype = -1;
		this->transhashcode = 0;
		this->clientuserappinfohashcode = 0;
		this->isRealTime = false;
	}
}UriParam;

typedef bool (SortFunc) (void* iterP1, void* iterP2);
typedef struct _table_title_info_t{
	std::string name;
	std::string hint;
	std::string href;
	std::string tdattr;
	SortFunc *ascFunc;
	SortFunc *descFunc;
} TableTitleInfo;
#define init_tableTitle_all(name, hint, href, tdattr, ascFunc, descFunc) {name, hint, href, tdattr, ascFunc, descFunc}
#define init_tableTitle_noattr(name, hint, href, ascFunc, descFunc) {name, hint, href, "", ascFunc, descFunc}
#define init_tableTitle_noattr_order(name, hint, href, func) {name, hint, href, "", func}
#define init_tableTitle_nofunc(name, hint, href, tdattr) {name, hint, href, tdattr, NULL, NULL}
#define init_tableTitle_noattrAndFunc(name, hint, href) {name, hint, href, "", NULL, NULL}
#define init_tableTitle_nohref(name, hint, tdattr) {name, hint, "", tdattr, NULL, NULL}
#define init_tableTitle(name, hint) {name, hint, "", "", NULL, NULL}
#define init_tableTitle_NULL(){"", "", "", "", NULL, NULL}
typedef std::vector<TableTitleInfo*> TableTitleInfoVec;

typedef struct _http_server_config_t{
	int pageSize;
	bool page_need_refresh;
	int page_refresh_time;//second
	bool current_page_home; //current page is home page or not

	bool show_topsqls;
	int home_topsqls_rows;
	bool show_clients;
	int home_clients_rows;
	bool show_taskThread;
	int home_taskThread_rows;

	//auto reset
	bool is_autoReset;
	int auto_reset_time;
	int reset_page_show_time;//second, default 5 second

	//real stat time
	int stats_interval; //second, default = 10s
	_http_server_config_t() {
		this->pageSize = 20;
		this->page_need_refresh = false;
		this->page_refresh_time = 5;
		this->current_page_home = false;

		this->show_topsqls = true;
		this->home_topsqls_rows = 5;
		this->show_clients = true;
		this->home_clients_rows = 5;
		this->show_taskThread = true;
		this->home_taskThread_rows = 5;

		this->is_autoReset = false;
		this->auto_reset_time = 0;
		this->reset_page_show_time = 5;

		this->stats_interval = 10;
	}
}HttpServerConfig;

class HttpResponse {
public:
	HttpResponse();
	~HttpResponse();

	int response_get(Http& http);

private:
	void response_setMenu(TopMenu &topMenu);

	void response_topclients(Http& http);
	void response_topwho(Http& http);
	void response_topsqls(Http& http);
	void response_toptables(Http& http);
	void response_toptablesMap(Http& http);
	void response_topTrans(Http& http);
	void response_findsql(Http& http);

	void response_home(Http& http);
	void response_machine(Http& http);
	void response_setting(Http& http);
	void response_saveSetting(Http& http);
	void response_reset(Http& http);

	void response_task(Http& http);
	void response_taskThread(Http& http);
	void response_lock(Http& http);
	void response_htmlPage(Http& http);//html page
	void response_dataBase(Http& http);

	void parse_uriParam(Http& http, UriParam& uriParam);
	void parse_httpServerConfig(Http& http);
	void gen_tableTitle(const TableTitleInfoVec& titleInfo, UriParam& uriParam, const char* caption, const int totalItem, Http& http);

	template<class S, int offset, bool asc>
	static bool compare(void* iterP1, void* iterP2);

	template<class VecType, class MapType, class mapIterType>
	void change_map2vecSort(const TableTitleInfoVec& titleInfo, const UriParam& uriParam, VecType& vec, MapType& mapobject);
	void clear_topMenu();

private:
	TopMenu topMenu;
	HttpHtml httpHtml;
	typedef void (HttpResponse::*Func) (Http &http);
	typedef std::map<std::string, Func> FuncMapType;
	FuncMapType responseFuncMap;
	HttpServerConfig httpServerConfig;
	MachineStatus machineStatus;
};

#endif /* HTTPSERVER_HTTPRESPONSE_H_ */
