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
* @FileName: ping.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月8日
*
*/

#include "ping.h"
#include "systemapi.h"
#include "memmanager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

Ping::Ping() {
	// TODO Auto-generated constructor stub

}

Ping::~Ping() {
	// TODO Auto-generated destructor stub
}

int Ping::decode_response(char *buf, int bytes, struct sockaddr_in *from) {

    IpHeader *iphdr;
    IcmpHeader *icmphdr;
    unsigned short iphdrlen;

    iphdr = (IpHeader *)buf; //获取IP报文首地址

    iphdrlen = (iphdr->ip_hl) * 4 ; //因为h_len是32位word，要转换成bytes必须*4

    if (bytes < iphdrlen + ICMP_MIN) {   //回复报文长度小于IP首部长度与ICMP报文最小长度之和，此时ICMP报文长不含 icmp_data
        logs(Logger::DEBUG, "Too few bytes from %s\n", inet_ntoa(from->sin_addr));
    }

    icmphdr = (IcmpHeader*)(buf + iphdrlen); //越过ip报头,指向ICMP报头

    //确保所接收的是我所发的ICMP的回应
    if (icmphdr->icmp_type != ICMP_ECHOREPLY) { //回复报文类型不是请求回显
        logs(Logger::DEBUG, "non-echo type %d recvd\n",icmphdr->icmp_type);
        return -1;
    }
    if (icmphdr->icmp_id != (u_uint16)SystemApi::get_pid()) { //回复报文进程号是否匹配
        logs(Logger::DEBUG,"someone else's packet!\n");
        return -1;
    }
    return 0;//ping success.
}

//计算ICMP首部校验和
u_uint16 Ping::checksum(u_uint16 *buffer, int size) {
    unsigned long cksum=0;
    //把ICMP报头二进制数据以2字节(16bit)为单位累加起来
    while(size >1) {
        cksum+=*buffer++;
        size -= 2;
    }
    //若ICMP报头为奇数个字节，会剩下最后一字节。把最后一个字节视为一个2字节数据的高字节，这个2字节数据的低字节为0，继续累加
    if(size) {
        cksum += *(u_uint8*)buffer;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff); //高16bit和低16bit相加
    cksum += (cksum >>16); //可能有进位情况，高16bit和低16bit再加1次
    return (u_uint16)(~cksum); //将该16bit的值取反，存入校验和字段
}
//
//设置ICMP报头
//
void Ping::fill_icmpData(char * icmp_data, int datasize){

    IcmpHeader *icmp_hdr;
    char *datapart;

    icmp_hdr = (IcmpHeader*)icmp_data;

    icmp_hdr->icmp_type = ICMP_ECHO; // ICMP报文类型为请求回显
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_id = (u_uint16)SystemApi::get_pid(); //获取当前的进程id
    icmp_hdr->icmp_cksum = 0;
    icmp_hdr->icmp_seq = 0;

    datapart = icmp_data + sizeof(IcmpHeader); //跳过IcmpHeader
    //
    // Place some junk in the buffer.
    //
    memset(datapart,'E', datasize - sizeof(IcmpHeader)); //填充datapart中的所有字节为"E"，长度为ICMP报文数据段长度
}

int Ping::ping(const char* pingAddress, int tryTimes, int packetSize)
{
	SystemSocket sockRaw = 0;
	struct sockaddr_in dest,from;
	struct hostent * hp;
	int bread, datasize;
	int fromlen = sizeof(from);
	char *icmp_data;
	char *recvbuf;
	unsigned int addr=0;
	u_uint16 seq_no = 0;
	int ret = 0;

	do {
		sockRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); ////创建套接字
		if ((int)sockRaw <= 0) {
			logs(Logger::ERR, "create socket error(%s)", SystemApi::system_strerror());
			sockRaw = 0;
			ret =  -1;
			break;
		}

		//set receve timeout
		if(SystemApi::system_setSocketRcvTimeo(sockRaw, 1, 0)) {
			logs(Logger::ERR,"failed to set recv timeout: %s\n", SystemApi::system_strerror());
			ret = -1;
			break;
		}

		//set send timeout
		if(SystemApi::system_setSocketSndTimeo(sockRaw, 1, 0)) {
			logs(Logger::ERR,"failed to set send timeout: %s\n", SystemApi::system_strerror());
			ret = -1;
			break;
		}
		memset(&dest,0,sizeof(dest)); //用0来填充一块大小为sizeof(dest)的内存区域

		hp = gethostbyname(pingAddress);
		if (!hp){
			addr = inet_addr(pingAddress); //inet_addr将IP地址从点数字符格式转换成网络字节格式整型。网络字节　7f   00   00   01 /主机字节　01   00   00   7f
		}
		if ((!hp) && (addr == INADDR_NONE) ) {
			logs(Logger::ERR, "Unable to resolve %s\n", SystemApi::system_strerror());
			ret = -1;
			break;
		}

		if (hp != NULL)
			memcpy(&(dest.sin_addr),hp->h_addr,hp->h_length);
		else
			dest.sin_addr.s_addr = addr;

		if (hp)
			dest.sin_family = hp->h_addrtype;
		else
			dest.sin_family = AF_INET; //AF_INET表示在Internet中通信

		datasize = packetSize;

		//
		//创建ICMP报文
		//
		datasize += sizeof(IcmpHeader);//发送数据包大小 + Icmp头大小
		icmp_data = (char*)MemManager::malloc(MAX_PACKET); //分配发包内存
		recvbuf = (char*)MemManager::malloc(MAX_PACKET);   //分配收包内存

		if (!icmp_data) {
			logs(Logger::ERR, "HeapAlloc failed %s\n", SystemApi::system_strerror());  //创建失败，打印提示信息
			ret = -1;
			break;
		}

		memset(icmp_data,0,MAX_PACKET); //把一个char a[20]清零, 就是 memset(a, 0, 20)
		fill_icmpData(icmp_data, datasize); ////初始化ICMP首部

		//send packet
		int i = 0;
		for(i = 0; i < tryTimes; i++){

			int bwrote;
			//初始化ICMP首部
			((IcmpHeader*)icmp_data)->icmp_cksum = 0; //校验和置零
			((IcmpHeader*)icmp_data)->icmp_data = SystemApi::system_millisecond();

			((IcmpHeader*)icmp_data)->icmp_seq = seq_no++; //序列号++
			((IcmpHeader*)icmp_data)->icmp_cksum = checksum((u_uint16*)icmp_data,datasize); //计算校验和

			do {
				bwrote = sendto(sockRaw, icmp_data, datasize, 0, (struct sockaddr*)&dest, sizeof(dest));  //发送数据
				if (bwrote != datasize){ //发送失败
					logs(Logger::ERR, "sendto failed: %s\n", SystemApi::system_strerror());
					ret = -1;
					break;
				}

				bread = recvfrom(sockRaw, recvbuf, MAX_PACKET, 0, (struct sockaddr*)&from, (socklen_t*)&fromlen);
				if (bread < 0){
//					logs(Logger::ERR, "recvfrom failed: %s\n", SystemApi::system_strerror());
					ret = -1;
					break;
				}

				if(decode_response(recvbuf,bread, &from)) {
					logs(Logger::DEBUG, "decode response error");
					ret = -1;
					break;
				}
			} while(0);

			if (ret < 0)
				ret = 0;
			else
				break;
		}
		if (i >= tryTimes) {
			ret = -1;
		}

		MemManager::free(icmp_data);
		MemManager::free(recvbuf);
	}while(0);

	if (sockRaw > 0) {
		SystemApi::close(sockRaw);
	}
	return ret;
}
