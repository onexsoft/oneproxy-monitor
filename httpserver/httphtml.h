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
* @FileName: httphtml.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月15日
*
*/

#ifndef HTTPSERVER_HTTPHTML_H_
#define HTTPSERVER_HTTPHTML_H_

#include "http.h"

#include <string>
#include <vector>
#include <iostream>

class Menu{
public:
	Menu(std::string menuName, std::string menuHref = "") {
		this->menuName = menuName;
		this->menuHref = menuHref;
	}

	void add_subMenu(Menu* subMenu) {
		this->subMenu.push_back(subMenu);
	}

	std::string gen_menu();

	int get_subMenuSize() {
		return this->subMenu.size();
	}

	std::vector<Menu*>& get_subMenu() {
		return this->subMenu;
	}

private:
	std::string menuName;
	std::string menuHref;
	std::vector<Menu*> subMenu;
};
typedef std::vector<Menu*> TopMenu;

class HttpHtml{
public:
	void response_setTitle(Http& http, std::string title = "OneProxy HTTP Server");
	void response_setMenu(Http& http, TopMenu menu);
	void response_endHtmlBody(Http& http);

private:
	void response_setStyle(Http& http);
};



#endif /* HTTPSERVER_HTTPHTML_H_ */
