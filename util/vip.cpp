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
* @FileName: vip.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月8日
*
*/

#include "vip.h"
#include "systemapi.h"
#include "logger.h"
#include "ping.h"

#ifndef _WIN32
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h> /* writev */
#include <arpa/inet.h> /** inet_ntoa */
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#else
#include <winsock2.h>
#define MSG_DONTWAIT 0x80
#endif
#include <string.h>

#define BROADCAST_PORT 3344

Vip::Vip(std::string ifName, std::string vipAddress, std::string threadName):
	Thread(thread_type_vip, threadName),
	m_ifname(ifName),
	m_vipAddress(vipAddress),
	m_isStop(false)
{
	// TODO Auto-generated constructor stub
	this->startThread(Vip::start, this);
}

Vip::~Vip() {
	// TODO Auto-generated destructor stub
}

int Vip::send_arp_packet(int ifndx, unsigned char* address, unsigned char* mac)
{
#ifndef _WIN32
	int err=0;
	int sfd = 0;
	unsigned char buf[256];
	struct ARP_PACKET *ah = (struct ARP_PACKET *)buf;
	struct sockaddr_ll dest;

	memset(buf,0,256);
	sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_RARP));
	if (sfd <= 0) {
		logs(Logger::ERR, "create socket error(%s)", SystemApi::system_strerror(SystemApi::system_errno()));
		return -1;
	}

	memset(&dest, 0, sizeof(dest));
	dest.sll_family = AF_PACKET;
	dest.sll_halen =  ETH_ALEN;
	dest.sll_ifindex = ifndx;
	memcpy(dest.sll_addr, mac, ETH_ALEN);

	ah->type     = htons(ETHERTYPE_ARP);
	memset(ah->dest_mac, 0xff, 6);
	memcpy(ah->src_mac, mac, 6);
	ah->hw_type  = htons(ARPHRD_ETHER);
	ah->pro_type = htons(ETH_P_IP);
	ah->hw_len   = ETH_ALEN;
	ah->pro_len  = 4;
	ah->op       = htons(ARPOP_REPLY);
	memcpy(ah->from_mac, mac, 6);
	memcpy(ah->from_ip, address, 4);
	memset(ah->to_ip, 0x00, 4);
	memset(ah->to_mac, 0xff, 6);

	err = sendto(sfd, buf, sizeof(*ah), 0, (struct sockaddr *)&dest, sizeof(dest));
	close(sfd);
	if (err != sizeof(*ah)){
		logs(Logger::ERR, "send to error(%s)", SystemApi::system_strerror(SystemApi::system_errno()));
		return -1;
	}
#endif
	return 0;
}

int Vip::send_broadcast(char* vipmac)
{
#ifndef _WIN32
	int socketfd = 0;
	struct sockaddr_in server_addr;
	int value = 1;
	if ((socketfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		logs(Logger::ERR, "create socket error(%s)", SystemApi::system_strerror());
		return -1;
	}
	setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, &value, sizeof(value));

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_BROADCAST;
	server_addr.sin_port = htons(BROADCAST_PORT);
	if(sendto(socketfd, vipmac, strlen(vipmac), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		logs(Logger::ERR, "send vipmac(%s) error(%s)", vipmac, SystemApi::system_strerror());
		close(socketfd);
		return -1;
	}
	close(socketfd);
#endif
	return 0;
}

int Vip::get_macAddresss(const char* ifname, char* mac, int macLen)
{
#ifndef _WIN32
	if (mac == NULL || ifname == NULL || macLen <= 0)
		return -1;

	int i = 0;
	int sfd = 0;
	struct ifreq ifr;
	sfd = socket (PF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
	 logs(Logger::ERR, "create socket error(%s)",SystemApi::system_strerror());
	 return -1;
	}

	i=0;
	while(ifname[i] && ifname[i] != ':') i++;
	if (ifname[i] == ':')
	{
	  memset(&ifr, 0, sizeof (ifr));
	  memcpy(ifr.ifr_name, ifname, i);
	  ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	  memset(&ifr.ifr_ifru, 0, sizeof(ifr.ifr_ifru));
	  if (ioctl (sfd, SIOCGIFHWADDR, &ifr) >= 0)
	  {
		  snprintf(mac, macLen, "%02x:%02x:%02x:%02x:%02x:%02x",
				  (0xff & ifr.ifr_hwaddr.sa_data[0]),
				  (0xff & ifr.ifr_hwaddr.sa_data[1]),
				  (0xff & ifr.ifr_hwaddr.sa_data[2]),
				  (0xff & ifr.ifr_hwaddr.sa_data[3]),
				  (0xff & ifr.ifr_hwaddr.sa_data[4]),
				  (0xff & ifr.ifr_hwaddr.sa_data[5]));
	  }
	}
	close(sfd);
#endif
	return 0;
}

int Vip::down_vipAddress(const char* vipIFName)
{
#ifndef _WIN32
	//1. judget the input is illege or not
	{
		if (vipIFName == NULL || strlen(vipIFName) <= 0){
			logs(Logger::ERR, "vipIFName is empty or null");
			return -1;
		}
	}

	//2. judge the vipIFName is eth0:0 format or not
	{
		unsigned int i = 0;
		while(vipIFName[i] != ':') i++;
		if (i >= strlen(vipIFName)){
			logs(Logger::ERR, "vipIFName is not correct format");
			return -1;
		}
	}

	//3. down the  interface
	{
		struct ifreq ifr;
		int sockfd = 0;
		sockfd = socket(PF_INET, SOCK_STREAM, 0);
		if (sockfd <= 0) {
			logs(Logger::ERR, "create socket error, errno: %d, errmsg: %s", errno, strerror(errno));
			return -1;
		}

		memset(&ifr, 0, sizeof (ifr));
		strncpy(ifr.ifr_name, vipIFName, IFNAMSIZ);
		ifr.ifr_name[IFNAMSIZ - 1] = '\0';
		ifr.ifr_flags = ~IFF_UP;
		ioctl (sockfd, SIOCSIFFLAGS, &ifr);
		close(sockfd);
	}
#endif
	return 0;
}

int Vip::set_vip_address(const char* ifname, const char* address, bool isdel)
{
#ifndef _WIN32
	int i = 0;
	int err = 0;
	int sfd = 0;
	struct ifreq ifr;
	char arpcmd[512];

	int    addr_mask = 0;
	struct sockaddr_in brdaddr;
	struct sockaddr_in netmask;

	int    ifindex = 0;
	struct sockaddr_in ipaddr;
	struct sockaddr macaddr;

	struct sockaddr_in *sockin = NULL;

	{
	  if(isdel) {
		  down_vipAddress(ifname);
		  return 0;
	  }
	}

	sfd = socket (AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		logs(Logger::ERR, "create socket error(%s)", SystemApi::system_strerror());
		return -1;
	}

	i=0;
	while(ifname[i] && ifname[i] != ':') i++;
	if (ifname[i] == ':')
	{
	  memset(&ifr, 0, sizeof (ifr));
	  memcpy(ifr.ifr_name, ifname, i);
	  ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	  addr_mask = 0;
	  memset(&ifr.ifr_ifru, 0, sizeof(ifr.ifr_ifru));
	  if (ioctl (sfd, SIOCGIFBRDADDR, &ifr) >= 0)
	  {
		  memcpy(&brdaddr, &ifr.ifr_broadaddr, sizeof(struct sockaddr_in));
		  addr_mask = addr_mask | 0x01;
	  }
	  memset(&ifr.ifr_ifru, 0, sizeof(ifr.ifr_ifru));
	  if (ioctl (sfd, SIOCGIFNETMASK, &ifr) >= 0)
	  {
		memcpy(&netmask, &ifr.ifr_netmask, sizeof(struct sockaddr_in));
		addr_mask = addr_mask | 0x02;
	  }
	  memset(&ifr.ifr_ifru, 0, sizeof(ifr.ifr_ifru));
	  if (ioctl (sfd, SIOCGIFHWADDR, &ifr) >= 0)
	  {
		  memcpy(&macaddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr_in));
		  addr_mask = addr_mask | 0x04;
	  }
	  memset(&ifr.ifr_ifru, 0, sizeof(ifr.ifr_ifru));
	  if (ioctl (sfd, SIOCGIFINDEX, &ifr) >= 0)
	  {
		ifindex = ifr.ifr_ifindex;
	  }
	  memset(&ifr.ifr_ifru, 0, sizeof(ifr.ifr_ifru));
	  if (ioctl (sfd, SIOCGIFADDR, &ifr) >= 0)
	  {
		memcpy(&ipaddr, &ifr.ifr_addr, sizeof(struct sockaddr_in));
		addr_mask = addr_mask | 0x08;
	  }
	}

	memset(arpcmd, 0, 512);
	sprintf(arpcmd, "/sbin/arping -A -c 1 -I %s %s > /dev/null 2>&1", ifr.ifr_name, address);

	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ifr.ifr_flags = ~IFF_UP;
	ioctl (sfd, SIOCSIFFLAGS, &ifr);
	memset(&ifr.ifr_ifru, 0, sizeof(ifr.ifr_ifru));
	if (isdel)
	{
		close(sfd);
		return 0;
	}

	sockin = (struct sockaddr_in *) &ifr.ifr_addr;
	sockin->sin_family = AF_INET;
	err = inet_aton(address, &sockin->sin_addr);
	netmask.sin_addr.s_addr = 0xffffffff;
	if (err)
	{
		 err = ioctl (sfd, SIOCSIFADDR, &ifr);
		 if (err < 0)
		 {
			close(sfd);
			return 0;
		 }

		 if (addr_mask & 0x1)
		 {
			memset(&ifr.ifr_ifru, 0, sizeof(ifr.ifr_ifru));
			memcpy(&ifr.ifr_broadaddr, &brdaddr, sizeof(struct sockaddr_in));
			ioctl (sfd, SIOCSIFBRDADDR, &ifr);
		 }
		 if (addr_mask & 0x2)
		 {
			memset(&ifr.ifr_ifru, 0, sizeof(ifr.ifr_ifru));
			memcpy(&ifr.ifr_netmask, &netmask, sizeof(struct sockaddr_in));
			ioctl (sfd, SIOCSIFNETMASK, &ifr);
		 }
		 close(sfd);

		 //add by huih@20160714
		 {
			 char macstr[32];
			 memset(macstr, 0, 32);
			 snprintf(macstr, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
					 (0xff & macaddr.sa_data[0]),
					 (0xff & macaddr.sa_data[1]),
					 (0xff & macaddr.sa_data[2]),
					 (0xff & macaddr.sa_data[3]),
					 (0xff & macaddr.sa_data[4]),
					 (0xff & macaddr.sa_data[5]));
			 send_broadcast(macstr);
		 }

		 sockin->sin_family = AF_INET;
		 err = inet_aton(address, &sockin->sin_addr);

		 int arpcnt = 10;
		 while(arpcnt--){
			 if (!send_arp_packet(ifindex, (unsigned char*)&sockin->sin_addr.s_addr, (unsigned char*)macaddr.sa_data)){
				 system((const char*)arpcmd);
			 }
		 }
		 return 1;
	}
	close(sfd);
#endif
	return 0;
}

int Vip::get_vipMacAddress(int bfid, char* mac, int macLen)
{

	if (mac == NULL || macLen <= 0 || bfid <= 0)
		return -1;

	struct sockaddr_in server_addr;
	socklen_t addr_len = sizeof(server_addr);
	if (recvfrom(bfid, mac, macLen, MSG_DONTWAIT, (sockaddr*)&server_addr, &addr_len) <= 0) {
		return -1;
	}
	return 0;
}

thread_start_func(Vip::start)
{
	//clear vip address
	Vip* vip = (Vip*)args;

	if(vip->get_ifname().length() <= 0) return 0;
	if (vip->get_vipAddress().length() <= 0) return 0;

	//clear vip address.
	if(vip->down_vipAddress(vip->get_ifname().c_str())) {
		logs(Logger::ERR, "down vip address error");
		return 0;
	}

	int serverfd = 0;
	do{
		struct sockaddr_in server_addr;
		socklen_t addr_len;
		if ((serverfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
			logs(Logger::ERR, "create socket error(%s)", SystemApi::system_strerror());
			return 0;
		}

		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_BROADCAST;
		server_addr.sin_port = htons(BROADCAST_PORT);
		addr_len = sizeof(server_addr);
		if (bind(serverfd, (struct sockaddr*)&server_addr, addr_len) < 0) {
			logs(Logger::ERR, "bind error(%s)", SystemApi::system_strerror());
			close(serverfd);
			serverfd = 0;
			return 0;
		}
	}while(0);

	char vipmac[32];
	char localmac[32] = {0};
	char gvipmac[32] = {0};
	Ping ping;

	//get local mac
	if (vip->get_macAddresss(vip->get_ifname().c_str(), localmac, 32)) {
		logs(Logger::ERR, "get local mac error");
		return 0;
	}

	while(vip->get_isStop() == false) {
		memset(vipmac, 0, 32);
		vip->get_vipMacAddress(serverfd, vipmac, 32);
		if (strlen(vipmac) > 0) {
			memcpy(gvipmac, vipmac, strlen(vipmac));
		}

		if ((strlen(gvipmac) <= 0 || strlen(gvipmac) != strlen(localmac)
				|| strncmp(gvipmac, localmac, strlen(gvipmac)) != 0)) {
			vip->set_vip_address(vip->get_ifname().c_str(), vip->get_vipAddress().c_str(), true);
		}

		if (ping.ping(vip->get_vipAddress().c_str())) {//ping 出现错误 或者ping不通
			vip->set_vip_address(vip->get_ifname().c_str(), vip->get_vipAddress().c_str(), false);
		}

		SystemApi::system_sleep(500);
	}

	return 0;
}
