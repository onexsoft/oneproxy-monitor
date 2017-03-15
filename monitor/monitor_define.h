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
 * @FileName: monitor_define.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月9日 下午5:34:21
 *  
 */

#ifndef MONITOR_DEFINE_H_
#define MONITOR_DEFINE_H_

#include <pcap.h>
#include <list>
#include <map>
#include <sstream>
#include "connection.h"

#ifdef linux
#include <netinet/in.h>
#endif
#include "stringbuf.h"

// tcp header
typedef struct _tcp_header_t{
	u_uint16 srcPort;
	u_uint16 desPort;
	u_uint32 sequenceNumber;
	u_uint32 ackNumber;

	u_uint8 th_offx2;
#define TH_OFF(th)      (((th->th_offx2) & 0xf0) >> 4)
	u_uint8 th_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	u_uint16 windowsSize;
	u_uint16 checkSum;
	u_uint16 urgentPointer;
}TcpHeader;

#define	ETHERTYPE_PUP		0x0200
#define	ETHERTYPE_IP		0x0800
#define	ETHERTYPE_ARP		0x0806
#define	ETHERTYPE_REVARP	0x8035

#define	ETHER_ADDR_LEN		6

struct	ether_header {
	u_int8_t	ether_dhost[ETHER_ADDR_LEN];
	u_int8_t	ether_shost[ETHER_ADDR_LEN];
	u_int16_t	ether_type;
};

struct vlan_8021q_header {
	u_int16_t	priority_cfi_vid;
	u_int16_t	ether_type;
};

/*
 * Definitions for internet protocol version 4.
 * Per RFC 791, September 1981.
 */
#define	IPVERSION	4

/*
 * Structure of an internet header, naked of options.
 *
 * We declare ip_len and ip_off to be short, rather than u_short
 * pragmatically since otherwise unsigned comparisons can result
 * against negative integers quite easily, and fail in subtle ways.
 */
struct ip {
	u_int8_t	ip_vhl;		/* header length, version */
#define IP_V(ip)	(((ip)->ip_vhl & 0xf0) >> 4)
#define IP_HL(ip)	((ip)->ip_vhl & 0x0f)
	u_int8_t	ip_tos;		/* type of service */
	u_int16_t	ip_len;		/* total length */
	u_int16_t	ip_id;		/* identification */
	u_int16_t	ip_off;		/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	u_int8_t	ip_ttl;		/* time to live */
	u_int8_t	ip_p;		/* protocol */
	u_int16_t	ip_sum;		/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
};

/*
 * Ethernet types.
 *
 * We wrap the declarations with #ifdef, so that if a file includes
 * <netinet/if_ether.h>, which may declare some of these, we don't
 * get a bunch of complaints from the C compiler about redefinitions
 * of these values.
 *
 * We declare all of them here so that no file has to include
 * <netinet/if_ether.h> if all it needs are ETHERTYPE_ values.
 */

#ifndef ETHERTYPE_PUP
#define	ETHERTYPE_PUP		0x0200	/* PUP protocol */
#endif
#ifndef ETHERTYPE_IP
#define	ETHERTYPE_IP		0x0800	/* IP protocol */
#endif
#ifndef ETHERTYPE_ARP
#define ETHERTYPE_ARP		0x0806	/* Addr. resolution protocol */
#endif
#ifndef ETHERTYPE_REVARP
#define ETHERTYPE_REVARP	0x8035	/* reverse Addr. resolution protocol */
#endif
#ifndef ETHERTYPE_NS
#define ETHERTYPE_NS		0x0600
#endif
#ifndef	ETHERTYPE_SPRITE
#define	ETHERTYPE_SPRITE	0x0500
#endif
#ifndef ETHERTYPE_TRAIL
#define ETHERTYPE_TRAIL		0x1000
#endif
#ifndef	ETHERTYPE_MOPDL
#define	ETHERTYPE_MOPDL		0x6001
#endif
#ifndef	ETHERTYPE_MOPRC
#define	ETHERTYPE_MOPRC		0x6002
#endif
#ifndef	ETHERTYPE_DN
#define	ETHERTYPE_DN		0x6003
#endif
#ifndef	ETHERTYPE_LAT
#define	ETHERTYPE_LAT		0x6004
#endif
#ifndef ETHERTYPE_SCA
#define ETHERTYPE_SCA		0x6007
#endif
#ifndef ETHERTYPE_REVARP
#define ETHERTYPE_REVARP	0x8035
#endif
#ifndef	ETHERTYPE_LANBRIDGE
#define	ETHERTYPE_LANBRIDGE	0x8038
#endif
#ifndef	ETHERTYPE_DECDNS
#define	ETHERTYPE_DECDNS	0x803c
#endif
#ifndef	ETHERTYPE_DECDTS
#define	ETHERTYPE_DECDTS	0x803e
#endif
#ifndef	ETHERTYPE_VEXP
#define	ETHERTYPE_VEXP		0x805b
#endif
#ifndef	ETHERTYPE_VPROD
#define	ETHERTYPE_VPROD		0x805c
#endif
#ifndef ETHERTYPE_ATALK
#define ETHERTYPE_ATALK		0x809b
#endif
#ifndef ETHERTYPE_AARP
#define ETHERTYPE_AARP		0x80f3
#endif
#ifndef	ETHERTYPE_8021Q
#define	ETHERTYPE_8021Q		0x8100
#endif
#ifndef ETHERTYPE_IPX
#define ETHERTYPE_IPX		0x8137
#endif
#ifndef ETHERTYPE_IPV6
#define ETHERTYPE_IPV6		0x86dd
#endif
#ifndef ETHERTYPE_PPP
#define	ETHERTYPE_PPP		0x880b
#endif
#ifndef	ETHERTYPE_MPLS
#define	ETHERTYPE_MPLS		0x8847
#endif
#ifndef	ETHERTYPE_MPLS_MULTI
#define	ETHERTYPE_MPLS_MULTI	0x8848
#endif
#ifndef ETHERTYPE_PPPOED
#define ETHERTYPE_PPPOED	0x8863
#endif
#ifndef ETHERTYPE_PPPOES
#define ETHERTYPE_PPPOES	0x8864
#endif
#ifndef	ETHERTYPE_LOOPBACK
#define	ETHERTYPE_LOOPBACK	0x9000
#endif

typedef struct task_t{
	struct pcap_pkthdr pkthdr;
	StringBuf* packet;
}TaskT;
typedef std::list<TaskT*> TaskList;

typedef struct _task_data_key{
	std::string srcIp;
	int srcPort;
	std::string desIp;
	int desPort;
	std::string cmpKey;

	void gen_cmpKey() {
		std::stringstream ss;
		ss << srcIp;
		ss << srcPort;
		ss << desIp;
		ss << desPort;
		this->cmpKey = ss.str();
	}

	bool operator < (const _task_data_key& data) const {
		return this->cmpKey < data.cmpKey;
	}
}TaskDataKeyT;

typedef struct _progress_info_t{
	int port;
	std::string name;
}ProgressInfo;

typedef enum _conn_flag_t{
	CONN_INIT,
	CONN_TCP,
	CONN_TCP_SYN,
	CONN_TCP_FIN,
	CONN_UDP,
}ConnFlag;

typedef struct task_data_t{
	struct timeval ts;
	TaskDataKeyT key;
	ProgressInfo pi;
	StringBuf data;
//	ConnFlag flag;
	u_uint8 tcpFlags;
	u_uint8 protocol;
}TaskDataT;
typedef std::list<TaskDataT*> TaskDataList;
typedef std::map<TaskDataKeyT, TaskDataList> TaskDataMap;

typedef void (*MonitorFunc)(void*, void*);
typedef std::map<int, bool> IntBoolMap;
typedef std::map<int, void*> IntVoidMap;
typedef std::map<std::string, void*> StringVoidMap;
typedef std::map<int, std::string> IntStringMap;
typedef std::map<unsigned long, std::string>ULongStringMap;

typedef std::map<TaskDataKeyT, Connection*> TaskDataKeyConnMap;
#endif /* MONITOR_DEFINE_H_ */
