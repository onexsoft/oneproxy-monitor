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
 * @FileName: testtcpclient.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年9月27日 上午11:36:44
 *  
 */

#include "testtcpclient.h"
#include "tcpclient.h"

void TestTcpClient::test_getBackendConnection() {

	TcpClient tcpClient;
	NetworkSocket  ns(std::string("127.0.0.1"), 1433);

	SystemApi::init_networkEnv();
	if (tcpClient.get_backendConnection(&ns)) {
		TEST_ASSERT(false);
		return;
	}
	ns.closeSocket(ns.get_fd());
	SystemApi::clear_networkEnv();

	TEST_ASSERT(true);
}

void TestTcpClient::test_sqlserver()
{
	TcpClient tcpClient;
	SystemApi::init_networkEnv();

	NetworkSocket ns(std::string("127.0.0.1", 1433));
	if (tcpClient.get_backendConnection(&ns)) {
		TEST_ASSERT(false);
		return;
	}





	ns.closeSocket(ns.get_fd());
	SystemApi::clear_networkEnv();




	TEST_ASSERT(true);
}
