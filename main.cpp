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
* @FileName: main.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月29日
*
*/
#include <iostream>

#include "ping.h"
#include "logger.h"
#include "config.h"
#include "keepalive.h"
#include "connection.h"
#include "connectmanager.h"

int main(int argc, char* argv[]) {
	//1. 处理参数
	config()->handle_args(argc, argv);

	//2. 根据需要keepalive
	if (config()->get_keepAlive()) {
		KeepAlive keepAlive;
		int child_exit_status = EXIT_SUCCESS;
		int ret = keepAlive.keepalive(&child_exit_status, config()->get_pidFilePath().c_str());
		if (ret == 0) {
			//the agent stopped
			return 0;
		} else uif (ret < 0) {
			return -1;
		}
		//else we are the child, go on
	}

	//3. 初始化网络环境
	uif(SystemApi::init_networkEnv()) {
		logs(Logger::ERR, "init network environment error, exit(-1)");
		return -1;
	}

	//4. start to handle client data.
	ConnectManager connectManager(config()->get_threadNum());
	connectManager.start();

	//5. 卸载网络环境
	SystemApi::clear_networkEnv();

	logs(Logger::ERR, "success finished ...");

	return 0;
}
