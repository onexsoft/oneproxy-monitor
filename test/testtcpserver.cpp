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
 * @FileName: testtcpserver.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年9月30日 上午11:16:28
 *  
 */

#include "testtcpserver.h"
#include "oneproxyserver.h"
#include "connectmanager.h"
#include "systemapi.h"
#include "sspacket.h"
#include "sslogin.h"

void TestTcpServer::test_server() {
	SystemApi::init_networkEnv();

	config()->handle_args(0, NULL);
	OneproxyServer tcpServer(std::string("0.0.0.0"), 8888);
	tcpServer.create_servers();
	NetworkSocket *cs = NULL;

	std::vector<NetworkSocket>::iterator it = tcpServer.servSct.begin();
	while(1){
		cs = tcpServer.accept_connect(it->get_fd());
		if (cs == NULL) {
			logs(Logger::FATAL, "accept error", NULL);
			return;
		}

		SSLogin login;
		if(login.login_init(INIT_SERVER_ENV)) {
			logs(Logger::FATAL, "init login error");
			return ;
		}

		LoginParam loginParam;
		std::string userName = std::string("sa");
		std::string password = std::string("0000");
		std::string defaultDBName = std::string("master");
		loginParam.init_loginClientParam(*cs, userName, password,
				defaultDBName, 4096, 0x0c, 0x00, 0x07, 0xd0, verTDS74);

		uif(!login.login_client(loginParam)) {
			loginParam.print();
			system("pause");
			break;
		}
	}

	cs->closeSocket(cs->get_fd());
	it->closeSocket(it->get_fd());
	SystemApi::clear_networkEnv();

}
