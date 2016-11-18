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

thread_start_func(AssistThread::start)
{
	AssistThread* self = (AssistThread*)args;

	ConnectionPool::get_pool().set_checkActive();
	ConnectionPool::get_pool().set_idleTimeoutCheck(config()->get_poolConnCheckActiveTime());
	ConnectionPool::get_pool().set_idleTimeoutRelease(config()->get_poolConnTimeoutReleaseTime());

	while(false == self->get_stop()) {
		//1. check the socket is active in pool
		ConnectionPool::get_pool().check_connectActive();

		if (cmpdata(u_uint64, (SystemApi::system_second() - record()->bakRecordStartTime),
				>=, record()->realRecordTime)) {
			record()->bak_record();
		}

		SystemApi::system_sleep(1000);
	}
	return 0;
}
