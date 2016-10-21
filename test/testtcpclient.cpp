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
#include "sslogin.h"
#include "conf/config.h"

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
#ifdef NOT_IN_GITHUB
	TcpClient tcpClient;
	SystemApi::init_networkEnv();

	config()->handle_args(0, NULL);

	NetworkSocket ns(std::string("127.0.0.1"), 9999);
	if (tcpClient.get_backendConnection(&ns)) {
		TEST_ASSERT(false);
		return;
	}

	/*ssl 的研究
	 * 1. 客户端向服务端发送：client hello (所支持的加密算法，随机数）
	 * 2. 服务端向客户端发送：server hello (所支持的加密算法），Certificate(证书），server hello done.
	 * 3. 客户端向服务端发送：ClientKeyExchange(加密后的master_secret)
	 *
	 * 1. 记录头信息包括：内容类型、长度和SSL版本。
	 * 2. ssl 支持四种内容类型：application_data,alert,handshake,change_cipher_spec
	 * 3. 所有数据都是以application_data类型来发送,alert主要用于报告各种类型的错误,handshake用于承载握手消息,change_cipher_spec消息表示记录加密及认证的改变
	 */
	SSLogin slogin;
	slogin.login_init(INIT_CLIENT_ENV);

	LoginParam loginParam;
	std::string hostName = std::string("huih");
	std::string appName = std::string("Microsoft JDBC Driver for SQL Server");
	std::string serverName = std::string("127.0.0.1");
	std::string ctlIntName = std::string("Microsoft JDBC Driver 4.0");
	std::string database = std::string("master");
	std::string userName = std::string("sa");
	std::string password = std::string("0000123");
	loginParam.init_loginServerParam(ns, userName, password, hostName, appName,
			serverName, ctlIntName, database, 8096, verTDS74);
	slogin.login_server(loginParam);
	loginParam.print();

	ns.closeSocket(ns.get_fd());
	SystemApi::clear_networkEnv();




	TEST_ASSERT(true);
#endif
}
