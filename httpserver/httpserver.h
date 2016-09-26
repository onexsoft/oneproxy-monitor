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
* @FileName: httpserver.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月11日
*
*/

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include "tcpserver.h"
#include "thread.h"
#include "ioevent.h"
#include "httpresponse.h"

#include <map>

class HttpServer : public TcpServer, public Thread{
public:
	HttpServer(std::string serverAddr, unsigned int serverPort, std::string threadName);
	virtual ~HttpServer();
	virtual void accept_clientRequest(NetworkSocket *clientSocket);
	void stop();

private:
	static thread_start_func(start);
	static void client_request(unsigned int fd, unsigned int events, void* args);
	void client_finishedRequest(unsigned int fd);
	void http_handle_request(NetworkSocket *ns);

private:
	typedef std::map<int, NetworkSocket*> HttpClientMap;//key: fd
	HttpClientMap httpClientMap;
	HttpResponse httpResponse;
	bool isStop;
};

#endif /* HTTPSERVER_H_ */
