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
* @FileName: httpserver.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月11日
*
*/

#include "httpserver.h"
#include "httpparse.h"
#include "record.h"

HttpServer::HttpServer(std::string serverAddr, unsigned int serverPort, std::string threadName)
	:TcpServer(serverAddr, serverPort),
	 Thread(thread_type_httpserver, threadName),
	 isStop(false)
{
	this->startThread(HttpServer::start, this);
}

HttpServer::~HttpServer()
{
	HttpClientMap::iterator it = this->httpClientMap.begin();
	for (; it != this->httpClientMap.end(); ++it) {
		delete it->second;
		it->second = NULL;
	}
	this->httpClientMap.clear();
}

void HttpServer::accept_clientRequest(NetworkSocket* clientSocket)
{
	if (clientSocket == NULL)
		return;

	this->httpClientMap[clientSocket->get_fd()] = clientSocket;
	this->get_ioEvent().add_ioEventRead(clientSocket->get_fd(), HttpServer::client_request, this);

	record()->record_httpServerClientConnect();
}

void HttpServer::stop()
{
	this->isStop = true;
}

thread_start_func(HttpServer::start)
{
	HttpServer *hs = (HttpServer*)args;
	hs->create_tcpServer();
	while(hs->isStop == false){
		hs->run_server(500);
	}
	return 0;
}

void HttpServer::client_request(unsigned int fd, unsigned int events, void *args)
{
	HttpServer *hs = (HttpServer*)args;
	if (hs->get_ioEvent().is_readEvent(events)) {
		NetworkSocket *ns = hs->httpClientMap[fd];
		if (ns == NULL) {
			hs->client_finishedRequest(fd);
			return;
		}
		ns->get_recvData().clear();//clear other data.
		if (ns->read_data()) {
			logs(Logger::INFO, "read fd(%d) error", ns->get_fd());
			hs->client_finishedRequest(fd);
			return;
		}
		hs->http_handle_request(ns);
		record()->record_httpServerRequestPage();
	} else if (hs->get_ioEvent().is_writeEvent(events)) {

	} else {
		logs(Logger::ERR, "fd(%d) unkown epoll events(%d)", fd, events);
	}
}

void HttpServer::client_finishedRequest(unsigned int fd)
{
	this->get_ioEvent().del_ioEvent(fd);
	NetworkSocket *ns = this->httpClientMap[fd];
	if (ns == NULL) {
		return;
	}
	ns->closeSocket(fd);
	this->httpClientMap.erase(fd);
	delete ns;

	record()->record_httpServerClientCloseConnect();
}

void HttpServer::http_handle_request(NetworkSocket *ns) {
//	logs(Logger::ERR, "client request string: %s", ns->get_stringBuf()->addr());

	HttpParse hp;
	Http http;
	if (hp.parse_httpRequest(ns->get_recvData(), http)) {
		logs(Logger::ERR, "parse http request error");
		return ;
	}

	if (httpResponse.response_get(http)) {
		logs(Logger::ERR, "reponse get error");
		return;
	}

//	logs(Logger::ERR, "outputbuf: %s", http.outputBuf.addr());
	//写response to server
	if (ns->write_data(http.outputBuf)) {
		logs(Logger::ERR, "wirte data to http client(%d) error", ns->get_fd());
		return;
	}
}
