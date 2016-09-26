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
* @FileName: http.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月15日
*
*/

#ifndef HTTPSERVER_HTTP_H_
#define HTTPSERVER_HTTP_H_

#include <map>
#include <string>
#include <iostream>
#include <string.h>

#include "logger.h"
#include "stringbuf.h"

#ifdef __WIN32
#define CRLF "\r\n"
#else
#define CRLF "\n"
#endif

class Http
{
public:
	void print_http(){
		logs(Logger::ERR, "http method: %d, uri: %s, version: %s", httpMethod, uri.c_str(), httpversion.c_str());
		std::map<std::string, std::string>::iterator it = headerOptionMap.begin();
		for (; it != headerOptionMap.end(); ++it) {
			logs(Logger::ERR, "option key: %s, value: %s", it->first.c_str(), it->second.c_str());
		}

		std::map<std::string, std::string>::iterator pit = uriParamMap.begin();
		for (; pit != uriParamMap.end(); ++pit) {
			logs(Logger::ERR, "uri param key: %s, value: %s", pit->first.c_str(), pit->second.c_str());
		}
	}
	std::string get_uriParam(std::string key) {
		std::map<std::string, std::string>::iterator it = this->uriParamMap.find(key);
		if (it == this->uriParamMap.end()) {
			return std::string("");
		} else {
			return it->second;
		}
	}

	void set_uriParam(std::string key, std::string value) {
		this->uriParamMap[key] = value;
	}

	std::string get_headerOption(std::string key) {
		std::map<std::string, std::string>::iterator it = this->headerOptionMap.find(key);
		if (it == this->headerOptionMap.end()) {
			return std::string("");
		} else {
			return it->second;
		}
	}

	void set_responseStatus(int code, std::string reason) {
		this->responseCode = code;
		this->responseReason = reason;
	}

	bool http_response_needs_body() {
		return this->responseCode != 204 && this->responseCode != 304
				&& (this->responseCode < 100 || this->responseCode >= 200)
				&& this->httpMethod != Http::HTTP_METHOD_HEAD;
	}

	bool uri_startWith(std::string startStr) {
		bool result = false;
		if (strncmp(this->uri.c_str(), startStr.c_str(), startStr.length()) == 0) {
			result = true;
		}
		return result;
	}

	int send_response();
private:
	int gen_responseHeader();

public:
	typedef enum _http_method_{
		HTTP_METHOD_GET,
		HTTP_METHOD_POST,
		HTTP_METHOD_HEAD,
		HTTP_METHOD_PUT,
		HTTP_METHOD_DELETE,
		HTTP_METHOD_TRACE,
		HTTP_METHOD_CONNECT,
		HTTP_METHOD_OPTIONS,
		HTTP_METHOD_PATCH,
		HTTP_METHOD_UNKOWN
	}HttpMethod;
	HttpMethod httpMethod;

	std::string uri;
	std::string baseUri;
	std::map<std::string, std::string> uriParamMap;

	std::string httpversion;//for example:HTTP/1.0, major:1, minor:0
	int httpversionMajor;
	int httpversionMinor;

	std::map<std::string, std::string> headerOptionMap;

	//response
	int responseCode;
	std::string responseReason;

	StringBuf outputBuf;
private:
	std::string responseHeader;
};
#endif /* HTTPSERVER_HTTP_H_ */
