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
* @ClassName: tcpclient.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月4日
*
*/

#include "tcpclient.h"

TcpClient::TcpClient()
{

}

TcpClient::TcpClient(std::string address, int port)
{
}

int TcpClient::get_backendConnection(NetworkSocket *ns)
{
	//1. parse address
	if (ns->parse_address() < 0) {
		logs(Logger::ERR, "parse address error");
		return -1;
	}

	int cfd = 0;
	//2. create sock
	if ((cfd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		logs(Logger::ERR, "create socket error(%s)", strerror(errno));
		return -1;
	}

	//3. set socket option
	if (ns->set_sockCommonOpt(cfd, (ns->get_addr().sa.sa_family == AF_UNIX)) < 0) {
		logs(Logger::ERR, "set sock(%d) common option error(%s)", cfd, strerror(errno));
		ns->closeSocket(cfd);
		return -1;
	}

	//4. connection to server
	int res = 0;
	while(1) {
		res = ::connect(cfd, &ns->get_addr().sa, sizeof(ns->get_addr().sa));
		if (res < 0 && errno == EINTR)
			continue;
		else if (res < 0) {
			logs(Logger::ERR, "connect error(%s)", SystemApi::system_strerror());
			ns->closeSocket(cfd);
			return -1;
		}
		break;
	}

	//5. save cfd
	ns->set_fd(cfd);

	return 0;
}
