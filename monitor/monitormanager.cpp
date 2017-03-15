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
		captureData(MonitorManager::monitorData, this)
{
	// TODO Auto-generated constructor stub
	this->set_signalHandleFunc(&MonitorManager::handle_signal);
	if (config()->get_monitorAutoGetDesIp()) {
		if (config()->get_monitorDeviceName().length() > 0) {
			captureData.set_device(config()->get_monitorDeviceName());
			//base device name get local ip
			if(captureData.get_localIpBaseDeviceName()) {
				logs(Logger::WARNING, "get local ip failed.");
			}
		} else {//get device name base on linsten address
			captureData.set_localIp(config()->get_oneproxyAddr());
			if (captureData.get_deviceNameBaseLocalIp()) {
				logs(Logger::FATAL, "get device name error, exit.");
			}
		}
		if (captureData.get_localIpListBaseDeviceName()) {
			logs(Logger::WARNING, "get local ip list failed.");
		}
	} else {
		captureData.set_device(config()->get_monitorDeviceName());
		std::string allIp = std::string("0.0.0.0");
		std::string localIp = std::string("127.0.0.1");
		std::string listenIp = config()->get_oneproxyAddr();
		if (allIp != listenIp && localIp != listenIp) {
			captureData.add_localIp2List(listenIp);
		}
		std::string monitorAddr = config()->get_monitorCaptureAddress();
		size_t last_pos = 0;
		while(1) {
			size_t pos = monitorAddr.find_first_of(';', last_pos);
			if (pos == monitorAddr.npos) {
				pos = monitorAddr.length();
			}

			std::string lip = monitorAddr.substr(last_pos, pos - last_pos);
			lip = Tool::stringTrim(lip);
			if (!lip.empty()) {
				captureData.add_localIp2List(lip);
			}

			last_pos = pos + 1;
			if (last_pos >= monitorAddr.length()) {
				break;
			}
		}
	}
	logs(Logger::INFO, "deviceName: %s, localIp: %s",
			captureData.get_device().c_str(), captureData.get_localIp().c_str());

	for (int i = 0; i < config()->get_threadNum(); ++i) {
		ParseData* pd = new ParseData(this);
		if (pd != NULL) {
			this->m_parseDataVec.push_back(pd);
		}
	}

	for (int i = 0; i < config()->get_monitorThreadNum(); ++i) {
		SuperMonitor* sm = new SuperMonitor(this);
		if (sm != NULL) {
			sm->set_captureData(&this->captureData);
			this->m_superMonitorVec.push_back(sm);
		}
	}
}

MonitorManager::~MonitorManager() {
	// TODO Auto-generated destructor stub
	std::vector<SuperMonitor*>::iterator sit = this->m_superMonitorVec.begin();
	for(; sit != this->m_superMonitorVec.end(); ++sit) {
		SuperMonitor* sm = (*sit);
		++sit;
		if (sm) {
			sm->joinThread();
			delete sm;
			sm = NULL;
		}
	}

	std::vector<ParseData*>::iterator it = this->m_parseDataVec.begin();
	for(; it != this->m_parseDataVec.end();) {
		ParseData* pd = (*it);
		++it;
		if (pd) {
			pd->joinThread();
			delete pd;
			pd = NULL;
		}
	}
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
	static int index = 0;
	static int smsize = mm->m_superMonitorVec.size();
	SuperMonitor* sm = mm->m_superMonitorVec.at((index++) % smsize);

	sm->monitorData((u_char*)sm, pkthdr, packet);
	if (MonitorManager::m_stop) {
		mm->captureData.stop_captureData();
		mm->stop_childThread();
	}
}

void MonitorManager::stop_childThread() {
	std::vector<SuperMonitor*>::iterator sit = this->m_superMonitorVec.begin();
	for(; sit != this->m_superMonitorVec.end(); ++sit) {
		SuperMonitor* sm = (*sit);
		sm->set_stop();
	}

	std::vector<ParseData*>::iterator it = this->m_parseDataVec.begin();
	for(; it != this->m_parseDataVec.end(); ++it) {
		ParseData* pd = (*it);
		pd->set_stop();
	}
}

ParseData* MonitorManager::get_parseData(int port) {
	static int size = this->m_parseDataVec.size();
	if (size > 0) {
		return this->m_parseDataVec.at(port%size);
	} else {
		return NULL;
	}
}

int MonitorManager::get_parseDataThreadSize() {
	return this->m_parseDataVec.size();
}
