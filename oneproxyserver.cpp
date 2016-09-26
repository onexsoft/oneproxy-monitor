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
* @ClassName: oneproxyserver.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月29日
*
*/

#include "tool.h"
#include "record.h"
#include "oneproxyserver.h"
#include "connectmanager.h"

int OneproxyServer::start_server()
{
	//1. 启动tcp server
	if(this->create_tcpServer()) {
		logs(Logger::ERR, "create tcp server error");
		return -1;
	}

	//2. run
	while(this->stop == false) {
		this->run_server(500);
	}

	return 0;
}

void OneproxyServer::accept_clientRequest(NetworkSocket *clientSocket)
{
	clientSocket->set_addressHashCode(
				Tool::quick_hash_code(clientSocket->get_address().c_str(), clientSocket->get_address().length()));
	record()->clientQueryMap[clientSocket->get_addressHashCode()].part.hashCode = clientSocket->get_addressHashCode();
	record()->clientQueryMap[clientSocket->get_addressHashCode()].part.connectNum ++;
	record()->clientQueryMap[clientSocket->get_addressHashCode()].ipAddr = clientSocket->get_address();
	record()->clientQueryMap[clientSocket->get_addressHashCode()].part.onLineStatus = true;
	record()->clientQueryMap[clientSocket->get_addressHashCode()].part.start_connect_time = SystemApi::system_millisecond();
	record()->clientQueryMap[clientSocket->get_addressHashCode()].latest_connect_time = (u_uint64)SystemApi::system_time();
	if (this->connectManager)
		this->connectManager->add_task(clientSocket);

}
