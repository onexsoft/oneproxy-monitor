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
 * @FileName: capturedata.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月8日 下午4:57:04
 *  
 */

#include "capturedata.h"
#include "systemapi.h"
#include "logger.h"

#ifdef __WIN32
#include <WinSock2.h>
#include <Iphlpapi.h>
#endif

CaptureData::CaptureData(DataHandleFunc func, void* funcArgs, std::string device, const int maxPacketSize) {
	// TODO Auto-generated constructor stub
	this->m_func = func;
	this->m_func_args = funcArgs;
	this->m_maxPacketSize = maxPacketSize;
	this->m_pcapHandle = NULL;
	this->m_pdumperHandle = NULL;
}

CaptureData::~CaptureData() {
	// TODO Auto-generated destructor stub
	if (this->m_pdumperHandle != NULL) {
		pcap_dump_close(this->m_pdumperHandle);
		this->m_pdumperHandle = NULL;
	}
	if (this->m_pcapHandle) {
		pcap_close(this->m_pcapHandle);
	}
}

void CaptureData::start_captureData()
{
	char errbuf[PCAP_ERRBUF_SIZE];
	this->m_pcapHandle = pcap_open_live(this->m_device.c_str(), this->m_maxPacketSize, 1, 0, errbuf);//timeout 1000
	if (this->m_pcapHandle != NULL) {
		pcap_loop(this->m_pcapHandle, -1, this->m_func, (u_char *)this->m_func_args);
	} else {
		logs(Logger::FATAL, "open pcap error(errno: %d, %s), device: %s",
				SystemApi::system_errno(),
				SystemApi::system_strerror(), this->m_device.c_str());
	}
}

void CaptureData::stop_captureData() {
	if (this->m_pcapHandle)
		pcap_breakloop(this->m_pcapHandle);
}

void CaptureData::gen_dumpFile(const struct pcap_pkthdr *header, const u_char *packet)
{
	if (this->m_pdumperHandle == NULL) {
		this->m_pdumperHandle = pcap_dump_open(this->m_pcapHandle, "superMonitor.pcap");
	}
	if (this->m_pdumperHandle == NULL) {
		logs(Logger::ERR, "pcap_dump_open error(%s)", SystemApi::system_strerror());
		return;
	}
	pcap_dump((u_char*)this->m_pdumperHandle, header, packet);
	pcap_dump_flush(this->m_pdumperHandle);
}

int CaptureData::get_deviceNameBaseLocalIp() {
	if (this->m_localIp.empty()) {
		logs(Logger::ERR, "the local ip is empty, error");
		return -1;
	}

	std::string deviceName = SystemApi::system_getDeivceName(this->m_localIp);
	if (deviceName.empty()) {
		logs(Logger::ERR, "device name is empty, error");
		return -1;
	}

	this->m_device = deviceName;
	return 0;
}

int CaptureData::get_localIpListBaseDeviceName() {

	if (this->m_device.empty()) {
		logs(Logger::ERR, "device name is empty");
		return -1;
	}

	this->localIpList.clear();
	if (SystemApi::system_getIpList(this->m_device, this->localIpList)) {
		logs(Logger::ERR, "get ip list error");
		return -1;
	}

	if (this->localIpList.empty()) {
		logs(Logger::WARNING, "the local ip list is empty");
	}
	return 0;
}

void CaptureData::add_localIp2List(std::string ip) {
	this->localIpList.push_back(ip);
}

bool CaptureData::is_localIp(std::string ip) {
	if (ip.empty())
		return false;
	std::list<std::string>::iterator it = this->localIpList.begin();
	for(; it != this->localIpList.end(); ++it) {
		if ((*it) == ip) {
			return true;
		}
	}
	return false;
}

bool CaptureData::isEmpty_localIp() {
	return this->localIpList.empty();
}

int CaptureData::get_localIpBaseDeviceName()
{
	if (this->m_device.empty()){
		logs(Logger::ERR, "the device name is empty, error");
		return -1;
	}

	std::string tip = SystemApi::system_getIp(this->m_device);
	if (tip.empty()) {
		logs(Logger::ERR, "ip is empty, error");
		return -1;
	}

	this->m_localIp = tip;
	return 0;
}

std::string CaptureData::get_deviceName() {
	char errbuf[PCAP_ERRBUF_SIZE];
#ifdef linux
	char* device = pcap_lookupdev(errbuf);
	if (device == NULL) {
		logs(Logger::FATAL, "lookup device error(%s)", SystemApi::system_strerror());
		return std::string("");
	}
	return std::string(device);
#else
	pcap_if_t* alldevs = NULL;
	if (pcap_findalldevs(&alldevs, errbuf) < 0) {
		logs(Logger::FATAL, "find all devs error(%s)", SystemApi::system_strerror());
	}

	bpf_u_int32 mask, net;
	pcap_if_t* d = NULL;
	std::string device;
	for(d = alldevs; d; d = d->next) {
		if (pcap_lookupnet (d->name, &net, &mask, errbuf) < 0) {
			continue;
		}
		if (net <= 0 || mask <= 0)
			continue;
		device = std::string(d->name);
	}
	pcap_freealldevs(alldevs);

	return device;
#endif
}

std::string CaptureData::get_deviceNameBaseIp(std::string ip) {
	std::string deviceName;
#ifdef __WIN32
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		delete pIpAdapterInfo;
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		nRel=GetAdaptersInfo(pIpAdapterInfo,&stSize);
	}

	if (ERROR_SUCCESS == nRel)
	{
		while (pIpAdapterInfo && deviceName.empty())
		{
			IP_ADDR_STRING *pIpAddrString =&(pIpAdapterInfo->IpAddressList);
			do
			{
				std::string tip = std::string(pIpAddrString->IpAddress.String);
				if (tip == ip) {
					deviceName = std::string(pIpAdapterInfo->AdapterName);
					break;
				}
				pIpAddrString=pIpAddrString->Next;
			} while (pIpAddrString);

			if (!deviceName.empty()) {
				break;
			}
			pIpAdapterInfo = pIpAdapterInfo->Next;
		}
	}

	if (pIpAdapterInfo)
		delete pIpAdapterInfo;

#endif

	return deviceName;
}

void CaptureData::get_deviceLocalIp() {
	if (this->m_device.empty())
		return;

	this->m_localIp = SystemApi::system_getIp(this->m_device);
}
