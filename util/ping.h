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
* @FileName: ping.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月8日
*
*/

#ifndef UTIL_PING_H_
#define UTIL_PING_H_

#include "define.h"
#include "logger.h"

#define ICMP_ECHO 8 //发送的icmp_type  ICMP回显请求报文
#define ICMP_ECHOREPLY 0 //接收的icmp_type  ICMP回显应答报文

#define ICMP_MIN 8 // ICMP报文的最小长度是8字节(仅为首部，不包含icmp_data）

/* The IP header */
typedef struct _ip_t
{
    unsigned char ip_hl:4;   //  4位首部长度 （length of the header）
    unsigned char ip_v:4;   // IP版本号
    unsigned char ip_tos;  // 8位服务类型TOS
    unsigned short ip_len;   // 16位总长度（字节）（total length of the packet）
    unsigned short ip_id;   //   16位标识
    unsigned short ip_off;    // 3位标志位
    unsigned char ip_ttl;   // 8位生存时间 TTL
    unsigned char ip_p;  // 8位协议
    unsigned short ip_sum;   // 16位IP首部校验和
    unsigned long ip_src;// 32位源IP地址
    unsigned long ip_dst;  // 32位目的IP地址
}IpHeader;

//
// ICMP header
//
typedef struct icmp
{
    unsigned char icmp_type;// 8位类型
    unsigned char icmp_code;// 8位代码
    unsigned short icmp_cksum;// 16位校验和
    unsigned short icmp_id; //ID标识
    unsigned short icmp_seq;  // 报文序列号
    unsigned long icmp_data; // 时间戳 不属于标准的ICMP头，只是用来记录时间
}IcmpHeader;

#define STATUS_FAILED 0xFFFF //状态失败
#define DEF_PACKET_SIZE    32 //发送数据包大小
#define DEF_PACKET_NUMBER  4    /* 发送数据报的个数 */
#define MAX_PACKET 1024 //最大包

class Ping {
public:
	Ping();
	virtual ~Ping();

	void fill_icmpData(char*, int);
	u_uint16 checksum(u_uint16*, int);
	int decode_response(char*, int, struct sockaddr_in*);

	int ping(const char* pingAddress, int tryTimes = DEF_PACKET_NUMBER, int packetSize = DEF_PACKET_SIZE);

};

#endif /* UTIL_PING_H_ */
