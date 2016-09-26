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
* @FileName: http.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月15日
*
*/


#include "http.h"
#include <stdio.h>
#include "../util/systemapi.h"

int Http::gen_responseHeader()
{

	//first line
	char code[4];
	sprintf(code, "%d", this->responseCode);
	responseHeader.append(this->httpversion);
	responseHeader.append(" " + std::string(code));
	responseHeader.append(" " + this->responseReason + CRLF);

	bool is_keepalive = false;
	bool is_close = false;
	std::string tv;
	tv = this->get_headerOption("Connection");
	if (tv.compare("keep-alive") == 0) {
		is_keepalive = true;
	} else if (tv.compare("close") == 0)
		is_close = true;

	//response header option
	if (this->httpversionMajor == 1) {
		if (this->httpversionMinor >= 1) {
			//add date
			char date[50];
			struct tm *cur_p = SystemApi::system_timeTM();
			if (strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", cur_p) != 0) {
				responseHeader.append("Date:" + std::string(date) + CRLF);
			}
		}

		/*
		 * if the protocol is 1.0; and the connection was keep-alive
		 * we need to add a keep-alive header, too.
		 */
		if (this->httpversionMinor == 0 && is_keepalive) {
			responseHeader.append("Connection:" + std::string("keep-alive") + CRLF);
		}

		if ((this->httpversionMinor >= 1 || is_keepalive) && this->http_response_needs_body()) {
			//add Content-Length
			char buf[10];
			sprintf(buf, "%d", this->outputBuf.length());
			responseHeader.append("Content-Length:" + std::string(buf) + CRLF);
		}
	}

	if (this->http_response_needs_body()) {
		responseHeader.append("Content-Type:" + std::string("text/html; charset=ISO-8859-1") + CRLF);
	}

	if (is_close && is_keepalive == false) {
		responseHeader.append("Connection:" + std::string("close") + CRLF);
	}
	responseHeader.append(CRLF);

	return 0;
}

int Http::send_response()
{
	if (this->gen_responseHeader()) {
		logs(Logger::ERR, "gen response header error");
		return -1;
	}

	this->outputBuf.insert(0, this->responseHeader.c_str(), this->responseHeader.length());

	return 0;
}
