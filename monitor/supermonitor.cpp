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
 * @FileName: supermonitor.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月9日 下午3:33:23
 *  
 */

#include "logger.h"
#include "supermonitor.h"
#include "systemapi.h"
#include "monitormanager.h"
#include <iostream>
#include <string>
#include <iostream>

#ifdef linux
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif

SuperMonitor::SuperMonitor(MonitorManager* mm)
	:Thread(thread_type_manager, std::string("super_monitor")){
	// TODO Auto-generated constructor stub

	this->m_captureData = NULL;
	this->m_stop = false;
	this->m_monitorManager = mm;
	this->startThread(SuperMonitor::start, this);
}

SuperMonitor::~SuperMonitor() {
	// TODO Auto-generated destructor stub
	this->joinThread();
	while(this->taskList.size() > 0) {
		TaskT* tt = this->get_taskFromList();
		if (tt) {
			delete tt;
			tt = NULL;
		}
	}
}

void SuperMonitor::set_stop() {
	this->m_stop = true;
	this->taskLock.lock();
	this->taskLock.signal_mutexCond();
	this->taskLock.unlock();
}

void SuperMonitor::monitorData(u_char* arg, const pcap_pkthdr* pkthdr, const u_char* packet)
{
	if (pkthdr->len <= 0)
		return;

	TaskT *t = new TaskT();
	t->packet = new StringBuf();
	t->pkthdr = *pkthdr;
	if (t->packet != NULL)
		t->packet->append(packet, pkthdr->len);

	SuperMonitor* sm = (SuperMonitor*)arg;
	sm->taskLock.lock();
	sm->taskList.push_back(t);
	sm->taskLock.signal_mutexCond();
	sm->taskLock.unlock();
}

thread_start_func(SuperMonitor::start) {
	SuperMonitor *sm = (SuperMonitor*)args;

	while(sm->get_stop() == false || !sm->taskList.empty()) {
		if (!sm->taskList.empty()) {
			TaskT* t = sm->get_taskFromList();
			if (t != NULL) {
				sm->handle_task(t);
			}
		} else {
			sm->taskLock.lock();
			sm->taskLock.wait_mutexCond();
			sm->taskLock.unlock();
		}
	}
	return 0;
}

TaskT* SuperMonitor::get_taskFromList() {
	TaskT* t = NULL;
	this->taskLock.lock();
	if (!this->taskList.empty()) {
		t = this->taskList.front();
		this->taskList.pop_front();
	}
	this->taskLock.unlock();
	return t;
}

void SuperMonitor::handle_task(TaskT* task) {
	assert(task != NULL);
	assert(task->packet);
#define free_task() {delete task->packet; delete task; task = NULL;}

	u_char* packet = (u_char*)task->packet->addr();
	struct pcap_pkthdr* pkthdr = &task->pkthdr;
	struct ether_header* eptr = (struct ether_header*)packet;
	int ether_type = ntohs(eptr->ether_type);
	const unsigned char* payload = packet + sizeof(struct ether_header);

	if (config()->get_monitorDumpData()) {
		this->m_captureData->gen_dumpFile(pkthdr, packet);
	}

	if(ether_type == ETHERTYPE_8021Q) {
		struct vlan_8021q_header* vptr = (struct vlan_8021q_header*)payload;
		ether_type = ntohs(vptr->ether_type);
		payload += sizeof(struct vlan_8021q_header);
	}
	if (ether_type != ETHERTYPE_IP) {
		free_task();
		return;
	}

	struct ip* iptr = (struct ip*)(payload);
	TcpHeader *tcpHeader = (TcpHeader*)(((char*)iptr) + IP_HL(iptr) * 4);
	std::string srcIp = std::string(inet_ntoa(iptr->ip_src));
	std::string desIp = std::string(inet_ntoa(iptr->ip_dst));
	u_uint16 srcPort = ntohs(tcpHeader->srcPort);
	u_uint16 desPort = ntohs(tcpHeader->desPort);
	std::string broadAddr = std::string("255.255.255.255");
	if (desIp == broadAddr) {//remove broadcast packet.
		free_task();
		return;
	}

	if (tcpHeader->th_flags == TH_ACK) {
		free_task();
		return;
	}

	int thl = 0;
	if (iptr->ip_p == IPPROTO_TCP) {
		thl = TH_OFF(tcpHeader) * 4;
	} else if (iptr->ip_p == IPPROTO_UDP) {
		thl = 8;
	}

	u_char* data = (u_char*)((char*)tcpHeader + thl);
	int ipLen = ntohs(iptr->ip_len);
	int ipHeaderLen = IP_HL(iptr) * 4;
	int dataLen = ipLen - ipHeaderLen - thl;
	dataLen = pkthdr->len - (int)(data - packet);

	static int vs = this->m_monitorManager->get_parseDataThreadSize();
	if (vs > 0) {
		TaskDataT *tdt = new TaskDataT();
		tdt->ts = pkthdr->ts;
		tdt->protocol = iptr->ip_p;
		tdt->tcpFlags = tcpHeader->th_flags;
		tdt->key.desIp = desIp;
		tdt->key.srcIp = srcIp;
		tdt->key.desPort = desPort;
		tdt->key.srcPort = srcPort;
		tdt->data.append(data, dataLen);

		if (m_captureData->isEmpty_localIp()) {
			tdt->pi.port = 0;
			tdt->pi.name.clear();

			ParseData* pd = this->m_monitorManager->get_parseData(0);
			if(pd) {
				pd->add_taskData(tdt);
			}
		} else if (m_captureData->is_localIp(srcIp)) {//send
			tdt->pi.port = srcPort;
			ParseData* pd = this->m_monitorManager->get_parseData(desPort);
			if(pd) {
				pd->add_taskData(tdt);
			}
		} else if (m_captureData->is_localIp(desIp)){//recv
			tdt->pi.port = desPort;
			ParseData* pd = this->m_monitorManager->get_parseData(srcPort);
			if(pd) {
				pd->add_taskData(tdt);
			}
		} else {
			delete tdt;
		}
	}
	free_task();
#undef free_task
}
