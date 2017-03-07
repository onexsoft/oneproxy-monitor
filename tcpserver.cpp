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
* @ClassName: tcpserver.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月11日
*
*/

#include <string.h>
#include <stdio.h>

#include "tcpserver.h"

#include "record.h"
#include "util/logger.h"
#include "util/systemapi.h"
#include "util/tool.h"

TcpServer::TcpServer():
	ioEvent(IoEvent::get_instance("tcpServer_ioevent"))
{
	this->listenBackLog = 128;
	this->isStopServer = false;
}

TcpServer::TcpServer(std::string serverAddr, unsigned int serverPort):
	 ioEvent(IoEvent::get_instance("tcpServer_ioevent"))
{
	this->listenBackLog = 128;
	this->isStopServer = false;
	this->servSct.push_back(NetworkSocket(serverAddr, serverPort));
}

TcpServer::~TcpServer()
{
	if (this->ioEvent != NULL) {
		this->ioEvent->destory_instance();
		this->ioEvent = NULL;
	}
}

int TcpServer::create_tcpServer() {
	//1. create server socket;
	if (this->create_servers() < 0) {
		logs(Logger::ERR, "create server error");
		return -1;
	}

	//2. add server to epoll
	std::vector<NetworkSocket>::iterator it = this->servSct.begin();
	for (; it != this->servSct.end(); ++it) {
		if (it->get_fd() > 0) {
			this->ioEvent->add_ioEventAccept(it->get_fd(), TcpServer::accept_connect, this);
			this->fdPortMap[it->get_fd()].port = it->get_port();
			this->fdPortMap[it->get_fd()].address = it->get_address();
		}
	}
	return 0;
}

void TcpServer::set_tcpServer(std::string serverAddr, std::set<unsigned int>& portList)
{
	std::set<unsigned int>::iterator it = portList.begin();
	for (; it != portList.end(); ++it) {
		this->servSct.push_back(NetworkSocket(serverAddr, *it));
	}
}

void TcpServer::stop_tcpServer()
{
	std::vector<NetworkSocket>::iterator  it = this->servSct.begin();
	for (; it != this->servSct.end(); ++it) {
		it->closeSocket(it->get_fd());
	}
}

void TcpServer::set_listenBackLog(int backLog)
{
	this->listenBackLog = backLog;
}

int TcpServer::create_servers()
{
	std::vector<NetworkSocket>::iterator it = this->servSct.begin();
	for (; it != this->servSct.end(); ++it) {
		NetworkSocket& ns = (*it);

		//1. parse address
		if (ns.parse_address() < 0) {
			logs(Logger::ERR, "parse address error");
			return -1;
		}

		//2. create listen socket
		if (this->create_listenSocket(ns, (int)(ns.get_addr().sa.sa_family), &ns.get_addr().sa, sizeof(ns.get_addr().sa)) < 0) {
			logs(Logger::ERR, "create listen socket error, port: %u", it->get_port());
			return -1;
		}
	}
	return 0;
}

int TcpServer::create_listenSocket(NetworkSocket& ns, int af, const struct sockaddr *sa, int salen)
{
	int sfd = 0;

	if ((sfd = ::socket(PF_INET, SOCK_STREAM, 0)) <= 0) {
		logs(Logger::ERR, "create socket error");
		return -1;
	}

	if (af != AF_UNIX) {
		if (ns.set_sockReUseAddr(sfd) < 0) {
			logs(Logger::ERR, "set resueaddr error");
			ns.closeSocket(sfd);
			return -1;
		}
		if (ns.set_sockReUsePort(sfd) < 0) {
			logs(Logger::ERR, "set socket reuseport error");
			ns.closeSocket(sfd);
			return -1;
		}
	}

	if (bind(sfd, sa, salen) < 0) {
		int errorno = SystemApi::system_errno();
		logs(Logger::ERR, "bind socket(%d) to address error(%d:%s)", sfd, errorno, SystemApi::system_strerror(errorno));
		ns.closeSocket(sfd);
		return -1;
	}

	if (ns.set_sockCommonOpt(sfd, af == AF_UNIX) < 0) {
		logs(Logger::ERR, "set common option error");
		ns.closeSocket(sfd);
		return -1;
	}

	if (::listen(sfd, this->listenBackLog) < 0) {
		int errorno = SystemApi::system_errno();
		logs(Logger::ERR, "listen error(%d:%s)", errorno, SystemApi::system_strerror(errorno));
		ns.closeSocket(sfd);
		return -1;
	}

	ns.set_fd(sfd);

	return 0;
}

NetworkSocket* TcpServer::accept_connect(unsigned int sfd)
{
	struct sockaddr client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int cfd = 0;
	if ((cfd = accept(sfd, &client_addr, &client_addr_len)) < 0) {
		logs(Logger::ERR, "call accept error(%s)", SystemApi::system_strerror());
		return NULL;
	}

	uif(SystemApi::system_setSockNonblocking(cfd, true)) {
		logs(Logger::ERR, "set socket(%d) non blocking error(%s)", cfd, SystemApi::system_strerror());
		SystemApi::close(cfd);
		return NULL;
	}

	NetworkSocket *clientSocket = new NetworkSocket();

	clientSocket->addr_assign(&client_addr);
	clientSocket->addr_ntop();
	clientSocket->set_fd(cfd);
	clientSocket->get_attachData().set_listenPort(this->fdPortMap[sfd].port);
	clientSocket->get_attachData().set_listenAddr(this->fdPortMap[sfd].address);

	logs(Logger::INFO, "accept client fd(%d) address (%s) port(%d) listenPort(%d)", cfd,
			clientSocket->get_address().c_str(), clientSocket->get_port(),
			clientSocket->get_attachData().get_listenPort());

	return clientSocket;
}

void TcpServer::accept_connect(unsigned int fd, unsigned int events, void *args)
{
	TcpServer *ts = (TcpServer*)args;
	uif (ts == NULL)
		return;

	NetworkSocket *clientSocket = ts->accept_connect(fd);
	uif (clientSocket == NULL) {
		logs(Logger::ERR, "accept client error");
		return;
	}
	ts->accept_clientRequest(clientSocket);
}
