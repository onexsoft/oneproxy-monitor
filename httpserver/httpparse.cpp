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
* @FileName: httpparse.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月12日
*
*/

#include "httpparse.h"
#include "logger.h"

#include <string.h>

int HttpParse::parse_httpRequest(StringBuf& req, Http& http)
{
	if (this->parse_httpFirstLine(req, http)) {
		logs(Logger::ERR, "parse http first line error");
		return -1;
	}

	if (this->parse_httpUriParam(http)) {
		logs(Logger::ERR, "parse http uri param error");
		return -1;
	}

	if (this->parse_httpHeaderOption(req, http)) {
		logs(Logger::ERR, "parse http header option error");
		return -1;
	}

//	http.print_http();
	return 0;
}

int HttpParse::parse_httpFirstLine(StringBuf& req, Http& http)
{
	char* firstLineCurrPos = req.addr() + req.get_offset();
	char* firstLineEndPos = strstr(firstLineCurrPos, CRLF);
	if (firstLineEndPos == NULL) {
		logs(Logger::ERR, "from addr(%s) + offset(%d) get first line comment error",
				req.addr(), req.get_offset());
		return -1;
	}

	typedef struct _http_method_t{
		const char* methodName;
		Http::HttpMethod httpMethod;
	}Http_method_T;
	Http_method_T tmpHttpMethod[] = {
			{"GET", Http::HTTP_METHOD_GET},
			{"POST", Http::HTTP_METHOD_POST},
			{"HEAD", Http::HTTP_METHOD_HEAD},
			{"PUT", Http::HTTP_METHOD_PUT},
			{"DELETE", Http::HTTP_METHOD_DELETE},
			{"TRACE", Http::HTTP_METHOD_TRACE},
			{"CONNECT", Http::HTTP_METHOD_CONNECT},
			{"OPTIONS", Http::HTTP_METHOD_OPTIONS},
			{"PATCH", Http::HTTP_METHOD_PATCH},
			{NULL, Http::HTTP_METHOD_UNKOWN}
	};

	typedef enum _status_step{
		HTTP_METHOD_STEP,
		HTTP_URI_STEP,
		HTTP_VERSION_STEP,
		HTTP_FINISHED
	}StatusStep;
	StatusStep currentStep = HTTP_METHOD_STEP;

	while(firstLineCurrPos < firstLineEndPos) {
		char* itemEndPos = strstr(firstLineCurrPos, " ");
		if (itemEndPos > firstLineEndPos)
			itemEndPos = firstLineEndPos;

		int itemLen = itemEndPos - firstLineCurrPos;
		switch(currentStep) {
		case HTTP_METHOD_STEP:
		{
			if (itemEndPos > firstLineEndPos) {
				logs(Logger::ERR, "parse request method error, current request data is %s", req.addr());
				return -1;
			}
			int i = 0;
			for (i = 0; tmpHttpMethod[i].methodName != NULL; i++) {
				if ((((size_t)itemLen) == strlen(tmpHttpMethod[i].methodName))
						&& strncmp((const char*)firstLineCurrPos, tmpHttpMethod[i].methodName, (size_t)itemLen) == 0) {
					http.httpMethod = tmpHttpMethod[i].httpMethod;
					break;
				}
			}
			if (tmpHttpMethod[i].methodName == NULL) {
				logs(Logger::ERR, "parse http method error, current request data is %s", req.addr());
				return -1;
			}
			currentStep = HTTP_URI_STEP;
		}
			break;
		case HTTP_URI_STEP:
		{
			if (itemEndPos > firstLineEndPos) {
				logs(Logger::ERR, "Do have uri, current request data is %s", req.addr());
				return -1;
			}
			if (strstr(firstLineCurrPos, "/") > itemEndPos) {
				logs(Logger::ERR, "parse uri error, current request data is %s", req.addr());
				return -1;
			}
			http.uri = std::string(firstLineCurrPos, itemLen);
			currentStep = HTTP_VERSION_STEP;
		}
			break;
		case HTTP_VERSION_STEP:
		{
			if (itemEndPos > firstLineEndPos) {
				logs(Logger::ERR, "parse http version error, current request data is %s", req.addr());
				return -1;
			}

			if (strstr(firstLineCurrPos, "HTTP/") > itemEndPos) {
				logs(Logger::ERR, "parse http version error, current request data is %s", req.addr());
				return -1;
			}
			http.httpversion = std::string(firstLineCurrPos, itemLen);

			if (parse_httpVersion(http)) {
				logs(Logger::ERR, "parse http version error, http version is %s", http.httpversion.c_str());
				return -1;
			}

			currentStep = HTTP_FINISHED;
		}
			break;
		default:
			logs(Logger::ERR, "program error, current step is %d", currentStep);
			return -1;

		}
		firstLineCurrPos = firstLineCurrPos + itemLen + 1;
	}

	if(currentStep != HTTP_FINISHED) {
		logs(Logger::ERR, "parse first line error, current request data is %s", req.addr());
		return -1;
	}

	req.set_offset(firstLineCurrPos - req.addr() + 1);

	return 0;
}

int HttpParse::parse_httpHeaderOption(StringBuf& req, Http& http)
{
	char* headerOptionCurrPos = req.addr() + req.get_offset();
	char* headerOptionEndPos = strstr(headerOptionCurrPos, CRLF);
	while(headerOptionEndPos - headerOptionCurrPos > 0) {

		char* keyEndPos = strstr(headerOptionCurrPos, ": ");
		if (keyEndPos > headerOptionEndPos) {
			logs(Logger::ERR, "parse header option error, headerOptionCurrPos: %s", headerOptionCurrPos);
			return -1;
		}

		std::string key = std::string(headerOptionCurrPos, keyEndPos-headerOptionCurrPos);
		std::string value = std::string(keyEndPos + 2, headerOptionEndPos - keyEndPos - 2);
		http.headerOptionMap[key] = value;

		headerOptionCurrPos = headerOptionEndPos + 2;
		if (headerOptionCurrPos >= req.addr() + req.length())//finished
			break;

		headerOptionEndPos = strstr(headerOptionCurrPos, CRLF);
		if (headerOptionEndPos == NULL) {
			logs(Logger::ERR, "parse header option error, not found CRLF. headerOptionCurrPos: %s", headerOptionCurrPos);
			return -1;
		}
	}

	return 0;
}

int HttpParse::parse_httpUriParam(Http& http)
{
	char* uriStartPos = (char*)http.uri.c_str();
	char* paramCurrPos = strstr(uriStartPos, "?");
	if (paramCurrPos == NULL) {//no param
		http.baseUri = http.uri;
		return 0;
	} else {
		http.baseUri = std::string(uriStartPos, paramCurrPos);
	}
	paramCurrPos = paramCurrPos + 1;

	char* paramEndPos = strstr(paramCurrPos, "&");
	if (paramEndPos == NULL)
		paramEndPos = uriStartPos + http.uri.length();

	while(paramEndPos <= (uriStartPos + http.uri.length())) {
		char* keyEndPos = strstr(paramCurrPos, "=");
		if (keyEndPos >= paramEndPos) {
			logs(Logger::ERR, "find key end pos error, paramCurrPos: %s", paramCurrPos);
			return -1;
		}

		std::string key = std::string(paramCurrPos, keyEndPos - paramCurrPos);
		std::string value = std::string(keyEndPos + 1, paramEndPos - keyEndPos - 1);

		http.uriParamMap[key] = value;

		paramCurrPos = paramEndPos + 1;
		if (paramCurrPos >= uriStartPos + http.uri.length())
			break;

		paramEndPos = strstr(paramCurrPos, "&");
		if (paramEndPos == NULL)
			paramEndPos = uriStartPos + http.uri.length();
	}

	return 0;
}

int HttpParse::parse_httpVersion(Http &http)
{
	unsigned int pos = http.httpversion.find("/");
	if (pos >= http.httpversion.npos) {
		logs(Logger::ERR, "not find // in httpversion(%s)", http.httpversion.c_str());
		return -1;
	}

	http.httpversionMajor = atoi(http.httpversion.c_str() + pos + 1);
	pos = http.httpversion.find(".");
	if (pos >= http.httpversion.npos) {
		logs(Logger::ERR, "not find . in http version(%s)", http.httpversion.c_str());
		return -1;
	}

	http.httpversionMinor = atoi(http.httpversion.c_str() + pos + 1);

	return 0;
}

