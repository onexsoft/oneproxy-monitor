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
* @ClassName: oneproxyserver.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月29日
*
*/

#ifndef ONEPROXYSERVER_H_
#define ONEPROXYSERVER_H_

#include <iostream>
#include <string>

#include "define.h"
#include "tcpserver.h"

class ConnectManager;
class OneproxyServer:public TcpServer{
public:
	OneproxyServer(ConnectManager* connectManager) {
		this->connectManager = connectManager;
		this->stop = false;
	}

	OneproxyServer(ConnectManager* connectManager, int serverPort, std::string serverAddr)
		:TcpServer(serverAddr, serverPort)
	{
		this->connectManager = connectManager;
		this->stop = false;
	}

	void set_stop() {
		this->stop = true;
	}

	int start_server();
	virtual void accept_clientRequest(NetworkSocket *clientSocket);
private:
	ConnectManager *connectManager;
	bool stop;
};

#endif /* ONEPROXYSERVER_H_ */
