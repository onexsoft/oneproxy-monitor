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
* @FileName: vip.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月8日
*
*/

#ifndef UTIL_VIP_H_
#define UTIL_VIP_H_

#include "thread.h"
#include "define.h"

struct ARP_PACKET
{
 unsigned char dest_mac[6];
 unsigned char src_mac[6];
 unsigned short type;
 unsigned short hw_type;
 unsigned short pro_type;
 unsigned char hw_len;
 unsigned char pro_len;
 unsigned short op;
 unsigned char from_mac[6];
 unsigned char from_ip[4];
 unsigned char to_mac[6];
 unsigned char to_ip[4];
};

class Vip : public Thread{
public:
	Vip(std::string ifName, std::string vipAddress, std::string threadName);
	virtual ~Vip();

	static thread_start_func(start);
private:
	/**
	 * @desc 发送关于address与mac的arp包到局域网中
	 * @param ifndex 接口的下标
	 *        address需要绑定的ip地址
	 *        mac 需要绑定的mac地址
	 * @return 成功返回0， 否则返回-1.
	 * **/
	int send_arp_packet(int ifndx, unsigned char* address, unsigned char* mac);

	/**
	 * @desc 广播当前绑定vip的主机的mac地址
	 * @param vipmac 绑定vip的主机的mac地址
	 * @return 成功返回0， 否则返回-1.
	 * **/
	int send_broadcast(char* vipmac);

	/**
	 * @desc 获取主机的mac地址
	 * @param ifname 网卡名称
	 *        mac 用于保存获取到的mac地址
	 *        macLen mac缓存的长度
	 * @return 成功返回0， 否则返回-1.
	 * **/
	int get_macAddresss(const char *ifname, char* mac, int macLen);

	/**
	 * @desc 下线网卡
	 * @param vipIFName 虚拟网卡
	 * @return 下线成功则返回0， 否则返回-1.
	 * **/
	int down_vipAddress(const char *vipIFName);

	/**
	 * @desc 设置VIP地址
	 * @param ifname 需要设置虚拟网卡的网卡名称
	 *        address 虚拟网卡ip地址
	 *        isdel true表示删除vip，false表示设置vip。
	 * @return 设置成功返回0， 否则返回-1.
	 * **/
	int set_vip_address(const char *ifname, const char *address, bool isdel);

	/**
	 * @desc 从局域网中获取vip对应的mac地址
	 * @param bfid sock的id
	 *        mac 保存mac地址的缓存
	 *        macLen mac地址缓存的长度
	 * @return 成功返回0， 否则返回-1.
	 * **/
	int get_vipMacAddress(int bfid, char* mac, int macLen);

	declare_class_member(std::string, ifname);
	declare_class_member(std::string, vipAddress);
	declare_class_member(bool, isStop);
};

#endif /* UTIL_VIP_H_ */
