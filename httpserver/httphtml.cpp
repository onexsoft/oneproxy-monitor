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
* @FileName: httphtml.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月15日
*
*/

#include "httphtml.h"
#include <stdio.h>
#include <string.h>

std::string Menu::gen_menu()
{
	if (this->menuName.length() <= 0)
		return "";

	std::string resultString;

	char buf[1024];
	if (this->menuHref.length() > 0) {
		sprintf(buf, "<li id='submenu'> <a href='%s'>%s</a>", this->menuHref.c_str(), this->menuName.c_str());
	} else {
		sprintf(buf, "<li id='submenu'> <a>%s</a>", this->menuName.c_str());
	}
	resultString.append(std::string(buf));

	if (this->subMenu.size() <= 0) {
		resultString.append("</li>");
		return resultString;
	}

	//set sub menu
	resultString.append("<ul class='sub-menu'>");
	unsigned int i = 0;
	for (; i < this->subMenu.size(); ++i) {
		memset(buf, 0, 1024);
		if (subMenu[i]->menuHref.length() > 0) {
			sprintf(buf, "<li id='submenu'> <a href='%s'>%s</a></li>", subMenu[i]->menuHref.c_str(), subMenu[i]->menuName.c_str());
		} else {
			sprintf(buf, "<li id='submenu'> <a>%s</a></li>", subMenu[i]->menuName.c_str());
		}
		resultString.append(std::string(buf));
	}
	resultString.append("</ul></li>");

	return resultString;
}

void HttpHtml::response_setTitle(Http &http, std::string title)
{
//	http.outputBuf.append("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
//	http.outputBuf.append("<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"zh\"><head>");
	http.outputBuf.append("<html><head>");
	http.outputBuf.append("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>");
	http.outputBuf.append("<title>");
	http.outputBuf.append(title.c_str());
	http.outputBuf.append("</title></head>");
	this->response_setStyle(http);
}

void HttpHtml::response_setStyle(Http &http) {
	char* style = (char*)"<style>"
			"body {  font: 90%/150%  Tahoma, \"Trebuchet MS\",  Arial;"
			"        color: #333333;        background: #f0f0f0;        margin: 0px auto;"
			"        padding: 0px; overflow-y: scroll;}"
			".data {padding: 0;margin: 0;border: none;border-collapse:collapse;}"
			".data caption {padding: 0 0 5px 0;font: 12px Tahoma,Georgia,\"Times New Roman\","
			"   Times,serif;text-align: center;}"
			".data th {font: bold 12px Tahoma,Georgia,\"Times New Roman\",Times,serif;"
			"        border: 1px solid #4a6783; padding: 5px 5px 5px 5px;"
			"        background: #CAE8EA  no-repeat;}"
			".data td {border: 1px solid #4a6783;background: white;"
			"        font-size:12px; padding: 5px 5px 5px 5px;}"
			".data tr:hover td {background: #2a4763; color: white;}"
			"a {color: #0082FF;text-decoration: none;}"
			"a:visited {color: #0082FF;text-decoration: none;}"
			"a:hover {color: #0082FF;background:border-bottom: 1px solid #0C72A2;}"

			"#botmenu{margin:0px 0px 0px 0px;font-size: 14px;overflow:hidden;"
			"   background:#000000;border:1px solid #111;}"
			"#submenu {margin: 0px 0px;padding:0px 0px;height:35px;}"
			"#submenu ul {width: auto;float:left;list-style: none;margin: 0;    padding: 0 10px;}"
			"#submenu li {float: left;list-style: none;margin: 0;padding: 0;    color: #aaa; font-weight:400;}"
			"#submenu li a {color: #ccc;display: block;margin: 0;padding: 5px 10px 6px 10px;    "
			"   text-decoration: none;  position: relative;}"
			"#submenu li a:hover, #submenu li a:active, #submenu .current_page_item a  {"
			"   color: #fa3000; background:#000000;}"
			"#submenu li a.sf-with-ul {padding-right: 10px; }"
			"#submenu li ul li a, #submenu li ul li a:link, #submenu li ul li a:visited,"
			"#submenu li ul li ul li a, #submenu li ul li ul li a:link, #submenu li ul li ul li a:visited,"
			"#submenu li ul li ul li ul li a, #submenu li ul li ul li ul li a:link, #submenu li ul li ul li ul li a:visited {"
			"   color: #aaa;    width: 148px;   margin: 0;  padding: 5px 10px;"
			"   border-top:1px solid #3a3a3a;   position: relative; font-weight:400;}"
			"#submenu ul li ul li:first-child a,#submenu ul li ul li ul li:first-child a,#submenu ul li ul li ul li ul li:first-child a  {"
			"   border-top:none;}"
			"#submenu li li a:hover, #submenu li li a:active {color: #fa3000; background:#000000;}"
			"#submenu li ul {z-index: 9999; position: absolute; left: -999em;   height: auto;"
			"   width: 170px;margin: 0px 0px 0px 0px;padding: 5px 5px;background:#2d2d2d;"
			"   border:1px solid #111;}"
			"#submenu li ul a { width: 150px;}"
			"#submenu li ul a:hover, #submenu li ul a:active { }"
			"#submenu li ul ul {margin: -48px 0 0 181px;}"
			"#submenu li:hover ul ul, #submenu li:hover ul ul ul, #submenu li.sfHover ul ul, #submenu li.sfHover ul ul ul {"
			"   left: -999em;}"
			"#submenu li:hover ul, #submenu li li:hover ul, #submenu li li li:hover ul,"
			" #submenu li.sfHover ul, #submenu li li.sfHover ul, #submenu li li li.sfHover ul {"
			"   left: auto;}"
			"#submenu li:hover, #submenu li.sfHover { position: static;}"
			"form {margin:0px; padding:0px;} #inputs {width: 250px; height:20px}"
			"#buttons {width: 80px; height:20px}"
			"</style>";
	http.outputBuf.append(style);
}

void HttpHtml::response_setMenu(Http& http, TopMenu menu) {
	http.outputBuf.append("<body> <div id='botmenu'> <div id='submenu'> <ul id='web2feel' class='sfmenu'>");
	unsigned int i = 0;
	for (i = 0; i < menu.size(); ++i) {
		http.outputBuf.append(menu[i]->gen_menu().c_str());
	}
	http.outputBuf.append("</ul></div></div><center>");
}

void HttpHtml::response_endHtmlBody(Http &http) {
	http.outputBuf.append("<div>Power by OneXSoft (http://www.onexsoft.com)</div>");
	http.outputBuf.append("</center>");
	http.outputBuf.append("</body>");
	http.outputBuf.append("</html>");
}

