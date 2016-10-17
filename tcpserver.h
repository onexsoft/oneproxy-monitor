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
* @ClassName: tcpserver.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月11日
*
*/


#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include "networksocket.h"
#include "ioevent.h"
#include <vector>
#include <set>
#include <iostream>

class TcpServer{
public:
	TcpServer();
	TcpServer(std::string serverAddr, unsigned int serverPort);
	virtual ~TcpServer();
	virtual void accept_clientRequest(NetworkSocket *clientSocket) = 0;

	//提供接口，让外部控制tcp的运行
	int create_tcpServer();
	void run_server(int timeout);
	IoEvent& get_ioEvent() {return *ioEvent;}
	void set_tcpServer(std::string serverAddr, std::set<unsigned int>& portList);

private:
	int create_servers();
	int create_listenSocket(NetworkSocket& ns, int af, const struct sockaddr *sa, int salen);
	NetworkSocket* accept_connect(unsigned int sfd);
	static void accept_connect(unsigned int fd, unsigned int events, void* args);

private:
	std::vector<NetworkSocket> servSct;
	std::map<unsigned int, unsigned int> fdPortMap;
	IoEvent *ioEvent;
};

#endif /* TCPSERVER_H_ */
