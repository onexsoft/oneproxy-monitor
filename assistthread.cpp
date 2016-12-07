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
 * @FileName: assistthread.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年11月9日 下午1:38:53
 *  
 */

#include "assistthread.h"
#include "connectmanager.h"

AssistThread::AssistThread(ConnectManager* manager)
	:Thread(thread_type_manager, std::string("assistThread"))
{
	this->is_stop = false;
	this->manager = manager;
	this->startThread(AssistThread::start, this);
}

AssistThread::~AssistThread()
{

}

void AssistThread::stop()
{
	this->is_stop = true;
}

bool AssistThread::get_stop()
{
	return this->is_stop;
}

bool AssistThread::connect_dataBaseIsOk(std::string addr, unsigned int port)
{
	static TcpClient tcpClient;
	NetworkSocket* ns = new NetworkSocket(addr, port);
	if (tcpClient.get_backendConnection(ns)) {
		return false;
	}
	ns->closeSocket(ns->get_fd());
	return true;
}

void AssistThread::check_dataBaseActive()
{
	unsigned int dbgs = config()->get_dataBaseGroupSize();
	for (unsigned int i = 0; i < dbgs; ++i) {
		DataBaseGroup* dbg = config()->get_dataBaseGroup(i);
		std::vector<DataBase*>::iterator it = dbg->get_dbMasterGroupVec().begin();
		for (; it != dbg->get_dbMasterGroupVec().end(); ++it) {
			if (this->connect_dataBaseIsOk((*it)->get_addr(), (*it)->get_port())) {
				(*it)->set_isActive(true);
			} else {
				(*it)->set_isActive(false);
				logs(Logger::WARNING, "database (%s:%d) is offline",
						(*it)->get_addr().c_str(), (*it)->get_port());
			}
		}

		std::vector<DataBase*>::iterator sit = dbg->get_dbSlaveGroupVec().begin();
		for (; sit != dbg->get_dbSlaveGroupVec().end(); ++sit) {
			if (this->connect_dataBaseIsOk((*sit)->get_addr(), (*sit)->get_port())) {
				(*sit)->set_isActive(true);
			} else {
				(*sit)->set_isActive(false);
				logs(Logger::WARNING, "database (%s:%d) is offline",
										(*sit)->get_addr().c_str(), (*sit)->get_port());
			}
		}
	}
}

thread_start_func(AssistThread::start)
{
	AssistThread* self = (AssistThread*)args;

	ConnectionPool::get_pool().set_checkActive();
	ConnectionPool::get_pool().set_idleTimeoutCheck(config()->get_poolConnCheckActiveTime());
	ConnectionPool::get_pool().set_idleTimeoutRelease(config()->get_poolConnTimeoutReleaseTime());

	while(false == self->get_stop()) {

		//check database is active or not
		self->check_dataBaseActive();

		//update global time
		config()->update_globalSecondTime();

		//check the socket is active in pool
		ConnectionPool::get_pool().check_connectActive();

		if (cmpdata(u_uint64, (SystemApi::system_second() - record()->bakRecordStartTime),
				>=, record()->realRecordTime)) {
			record()->bak_record();
		}

		SystemApi::system_sleep(1000);
	}
	return 0;
}
