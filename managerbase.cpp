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
 * @FileName: managerbase.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月23日 下午2:50:22
 *  
 */

#include "managerbase.h"
#include "conf/config.h"
#include "pidmanager.h"
#include <string>
#include <iostream>
#include <signal.h>
#include <unistd.h>


ManagerBase::ManagerBase():
	vipThread(config()->get_vipIfName(), config()->get_vipAddress(), std::string("vipthread")),
	httpServer(config()->get_httpServerAddr(), config()->get_httpServerPort(), std::string("httpserver"))
{
	// TODO Auto-generated constructor stub
	this->func = NULL;
}

ManagerBase::~ManagerBase() {
	// TODO Auto-generated destructor stub
	vipThread.set_isStop(true);
	assistThread.stop();
	httpServer.stop();
}

void ManagerBase::start() {
	SystemApi::system_setThreadName("mainThread");

	//handle signal
	if (this->func) {
		signal(SIGTERM, this->func);
		signal(SIGINT,  this->func);
		signal(SIGUSR1, this->func);
		signal(SIGUSR2, this->func);
	}

	//close old process.
	if (config()->get_pidFilePath().size() > 0) {
		if (PidManager::handle_reboot(config()->get_pidFilePath().c_str())) {
			logs(Logger::ERR, "reboot error");
			return;
		}
	}

	if (config()->get_pidFilePath().size()) {
		if(PidManager::save_pid(config()->get_pidFilePath().c_str())) {
			logs(Logger::ERR, "write pid error");
			return;
		}
	}

	this->start_child();

	//stop stats
	record()->record_stop();

	// unlink pid file
	if (config()->get_pidFilePath().size()) {
		PidManager::unlink_pid(config()->get_pidFilePath().c_str());
	}
}

void ManagerBase::set_signalHandleFunc(SignalHandleFunc func)
{
	this->func = func;
}
