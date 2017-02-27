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
 * @FileName: monitormanager.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月23日 下午12:57:44
 *  
 */

#include "monitormanager.h"

bool MonitorManager::m_stop = false;
MonitorManager::MonitorManager() :
	sm(config()->get_threadNum()),
	captureData(MonitorManager::monitorData, this)
{
	// TODO Auto-generated constructor stub
	this->set_signalHandleFunc(&MonitorManager::handle_signal);
	sm.set_captureData(&captureData);
#ifdef __WIN32
	captureData.set_windowsLocalIp(config()->get_oneproxyAddr());
	captureData.set_windowDeviceName();
#endif
}

MonitorManager::~MonitorManager() {
	// TODO Auto-generated destructor stub
}

void MonitorManager::start_child() {
	captureData.start_captureData();
}

void MonitorManager::handle_signal(int sig)
{
	MonitorManager::m_stop = true;
}

void MonitorManager::monitorData(u_char* arg, const pcap_pkthdr* pkthdr, const u_char* packet)
{
	assert(arg);
	MonitorManager* mm = (MonitorManager*)arg;
	mm->sm.monitorData((u_char*)(&(mm->sm)), pkthdr, packet);
	if (MonitorManager::m_stop) {
		mm->captureData.stop_captureData();
		mm->sm.set_stop();
	}
}
