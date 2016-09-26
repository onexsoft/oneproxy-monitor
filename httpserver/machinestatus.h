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
* @FileName: machinestatus.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月28日
*
*/

#ifndef HTTPSERVER_MACHINESTATUS_H_
#define HTTPSERVER_MACHINESTATUS_H_

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "../util/systemapi.h"

//read load average
typedef struct LoadAverage_t{
	double load1m;//1分钟
	double load5m;//5分钟
	double load15m;//15分钟
	int procrun;//正在运行的进程数
	int procnum;//系统进程总数
}LoadAverage;

//read cpu info
typedef struct CpuInfo_t{
	double   user;
	double   nice;
	double   system;
	double   idle;
	double   iowait;
	double   irq;
	double   softirq;
	int      processes;
	int      context;
	int      runproc;
	int      blkproc;
	unsigned int uptime;
	double   irqcall;
}CpuInfo;

//read swap info
//top -n 1 | awk 'NR==5{print}' | awk '{print $2 $4 $6 $8}'
typedef struct PageSwap_t{
	unsigned int   pgpgin;
	unsigned int   pgpgout;
	unsigned int   pswpin;
	unsigned int   pswpout;
}PageSwap;

typedef struct MemoryInfo_t{
	double memtotal;
	double memfree;
	double buffers;
	double cached;
	double swapcached;
	double swaptotal;
	double swapfree;
	double lowtotal;
	double lowfree;
	double hightotal;
	double highfree;
	double pagetables;
}MemoryInfo;

typedef struct Network_t{
	double   ibytes;
	double   ipackets;
	double   ierrs;
	double   obytes;
	double   opackets;
	double   oerrs;
}Network;

typedef struct FileNR_t{
	unsigned int nropen;
	unsigned int nrtotal;
}FileNR;

typedef struct SocketStats_t{
	int   tcpuse;
	int   tcpalloc;
	int   tcpwait;
	int   udpuse;
}SocketStats;

typedef struct SnmpTcpProtocol_t{
	int   activeopen;
	int   pasiveopen;
	int   afails;
	int   eresets;
	int   insegs;
	int   outsegs;
	int   resegs;
	int   inerrs;
	int   outrst;
}SnmpTcpProtocol;

typedef struct CurrentProcessInfo_t{
	int pid;
	char user[32];
	int pr;
	int ni;
	char virt[16];
	char res[16];
	char shr[16];
	char status[2];
	double cpu;
	double mem;
	char time[64];
	char command[1024];
	CurrentProcessInfo_t() {
//		this->pid = 0;
//		memset(this->user, 0, sizeof(this->user));
//		this->pr = 0;
//		this->ni = 0;
//		memset(this->virt, 0, sizeof(this->virt));
//		memset(this->res, 0, sizeof(this->res));
//		memset(this->shr, 0, sizeof(this->shr));
//		memset(this->status, 0, sizeof(this->status));
//		this->cpu = 0;
//		this->mem = 0;
//		memset(this->time, 0, sizeof(this->time));
//		memset(this->command, 0, sizeof(this->command));
		memset(this, 0, sizeof(*this));
	}
}CurrentProcessInfo;

typedef struct OSLoadData_t{
	LoadAverage la;
	CpuInfo ci;
	PageSwap ps;
	MemoryInfo mi;
	Network nt;
	FileNR fnr;
	SocketStats ss;
	SnmpTcpProtocol stp;
	OSLoadData_t() {
		memset(this, 0, sizeof(*this));
	}
}OSLoadData;

typedef struct OSLoadRecord_t{
   double load;
   int   run;
   int   num;

   int   user;
   int   system;
   int   iowait;
   int   fork;
   int   irq;
   int   softirq;
   int   context;
   int   uptime;
   int   irqcall;

   int   pgin;
   int   pgout;
   int   swpin;
   int   swpout;

   double   free;
   double   swap;
   double   cache;
   double   lfree;
   double   hfree;
   double   pagetables;

   int   ibytes;
   int   ipackets;
   int   ierrors;
   int   obytes;
   int   opackets;
   int   oerrors;

   int   nr;
   int   nropen;
   int   nrtotal;

   int   tcpuse;
   int   tcpalloc;
   int   tcpwait;
   int   aopen;
   int   popen;
   int   afail;
   int   reset;
   int   isegs;
   int   osegs;
   int   rsegs;
   int   inerr;
   int   orest;
   OSLoadRecord_t() {
	   memset(this, 0, sizeof(*this));
   }
} OSLoadRecord;

class MachineStatus {
public:
	MachineStatus();
	virtual ~MachineStatus();
	void get_machineStatus(OSLoadRecord &rec);
	int get_systemCurrentProcessInfo(CurrentProcessInfo& cpi);

private:
	int get_systemLoadAverage(LoadAverage* la);
	int get_systemCpuInfo(CpuInfo* ci);
	int get_systemPageSwap(PageSwap* ps);
	int get_systemMemoryInfo(MemoryInfo* mi);
	int get_systemNetwork(Network* nt);
	int get_systemFileNR(FileNR *fnr);
	int get_systemSocketStats(SocketStats* ss);
	int get_systemSnmpTcpProtocol(SnmpTcpProtocol* stp);
	int get_systemInfo(OSLoadData* osd);

	int get_processName(char*pn);
	int get_processPid(char* pn);
	int intDouble(double a, double b);
	double doubleDouble(double a, double b);
	void getOSLoadRecord(OSLoadData *os1, OSLoadData *os2, OSLoadRecord* rec);
};

#endif /* HTTPSERVER_MACHINESTATUS_H_ */
