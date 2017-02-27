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
 * @FileName: capturedata.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月8日 下午4:57:04
 *  
 */

#ifndef CAPTUREDATA_H_
#define CAPTUREDATA_H_

#include "define.h"
#include <string>
#include <pcap.h>
using namespace std;

typedef void (*DataHandleFunc)(u_char *, const struct pcap_pkthdr *, const u_char *);

class CaptureData {
public:
	CaptureData(DataHandleFunc func, void* funcArgs, std::string device = std::string(), const int maxPacketSize = BUFSIZ);
	virtual ~CaptureData();

	void start_captureData();
	void stop_captureData();
	void gen_dumpFile(const struct pcap_pkthdr *header, const u_char *packet);
#ifdef __WIN32
	void set_windowsLocalIp(std::string ip);
	void set_windowDeviceName();
#endif
private:
	std::string get_deviceName();
	std::string get_deviceNameBaseIp(std::string ip);
	void get_deviceLocalIp();

private:
	declare_class_member(DataHandleFunc, func);
	declare_class_member(void*, func_args);
	declare_class_member(std::string, device);
	declare_class_member(int, maxPacketSize);
	declare_class_member(std::string, localIp);
	declare_class_member(pcap_t*, pcapHandle);
	declare_class_member(pcap_dumper_t*, pdumperHandle);
};

#endif /* CAPTUREDATA_H_ */
