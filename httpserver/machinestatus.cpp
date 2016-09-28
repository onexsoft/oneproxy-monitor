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
* @FileName: machinestatus.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月28日
*
*/

#include "machinestatus.h"
#include <string.h>
#include <math.h>
#include "../util/logger.h"

MachineStatus::MachineStatus() {
	// TODO Auto-generated constructor stub

}

MachineStatus::~MachineStatus() {
	// TODO Auto-generated destructor stub
}

void MachineStatus::get_machineStatus(OSLoadRecord& rec)
{
	OSLoadData ld1, ld2;
	this->get_systemInfo(&ld1);
	SystemApi::system_sleep(1000);
	this->get_systemInfo(&ld2);

	this->getOSLoadRecord(&ld1, &ld2, &rec);
}

int MachineStatus::get_systemLoadAverage(LoadAverage* la)
{
    FILE *file = NULL;

    file = fopen("/proc/loadavg", "r");
    if (file == NULL) {
        return -1;
    }
    fseek(file, 0, SEEK_SET);
    fscanf(file, "%lf %lf %lf %d/%d", &la->load1m, &la->load5m, &la->load15m, &la->procrun, &la->procnum);
    fclose(file);

    return 0;
}

int MachineStatus::get_systemCpuInfo(CpuInfo* ci) {

    FILE *file = NULL;

    file = fopen("/proc/stat", "r");
    if (file == NULL)
        return -1;

    char buf[1024];
    char cpu[10];

    fgets(buf, 1024, file);
    sscanf(buf,"%s %lf %lf %lf %lf %lf %lf %lf",cpu, &ci->user, &ci->nice, &ci->system, &ci->idle, &ci->iowait, &ci->irq, &ci->softirq);
    memset(buf, 0, sizeof(buf));
    while(fgets(buf, sizeof(buf), file) != NULL) {
        cpu[0] = 0;
        sscanf(buf, "%s", cpu);
        if (strcmp(cpu, "ctxt") == 0) {
            sscanf(buf, "%s %d", cpu, &ci->context);
        } else if (strcmp(cpu, "btime") == 0) {
            sscanf(buf, "%s %d", cpu, &ci->uptime);
        } else if (strcmp(cpu, "processes") == 0) {
            sscanf(buf, "%s %d", cpu, &ci->processes);
        } else if (strcmp(cpu, "procs_running") == 0) {
            sscanf(buf, "%s %d", cpu, &ci->runproc);
        } else if (strcmp(cpu, "procs_blocked") == 0) {
            sscanf(buf, "%s %d", cpu, &ci->blkproc);
        } else if (strcmp(cpu, "intr") == 0) {
            sscanf(buf, "%s %lf", cpu, &ci->irqcall);
        }
        memset(buf, 0, sizeof(buf));
    }

    fclose(file);

    return 0;
}

int MachineStatus::get_systemPageSwap(PageSwap* ps)
{
    FILE *file = NULL;
    file = fopen("/proc/vmstat", "r");
    if (file == NULL)
        return -1;

    char buf[1024];
    char name[128];
    unsigned int value;

    while(fgets(buf, 1024, file) != NULL) {
        sscanf(buf, "%s %d", name, &value);
        if (strcmp(name, "pgpgin") == 0) {
            ps->pgpgin = value;
        } else if (strcmp(name, "pgpgout") == 0) {
            ps->pgpgout = value;
        } else if (strcmp(name, "pswpin") == 0) {
            ps->pswpin= value;
        } else if (strcmp(name, "pswpout") == 0) {
            ps->pswpout = value;
        }
        name[0] = '\0';
    }
    fclose(file);

    return 0;
}

int MachineStatus::get_systemMemoryInfo(MemoryInfo* mi)
{
    FILE *fp = NULL;
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
        return -1;

    char buf[1024];
    char name[32];
    int value;
    while(fgets(buf, 1024, fp) != NULL) {
        name[0] = '\0';
        sscanf(buf, "%s %d", name, &value);

        if (strcmp(name, "MemTotal:") == 0) {
            mi->memtotal = value;
        } else if (strcmp(name, "MemFree:") == 0) {
            mi->memfree = value;
        } else if (strcmp(name, "Buffers:") == 0) {
            mi->buffers = value;
        } else if (strcmp(name, "Cached:") == 0) {
            mi->cached = value;
        } else if (strcmp(name, "SwapTotal:") == 0) {
            mi->swaptotal = value;
        } else if (strcmp(name, "SwapFree:") == 0) {
            mi->swapfree = value;
        } else if (strcmp(name, "SwapCached:") == 0) {
            mi->swapcached = value;
        } else if (strcmp(name, "LowTotal:") == 0) {
            mi->lowtotal = value;
        } else if (strcmp(name, "LowFree:") == 0) {
            mi->lowfree = value;
        } else if (strcmp(name, "HighTotal:") == 0) {
            mi->hightotal = value;
        } else if (strcmp(name, "HighFree:") == 0) {
            mi->highfree = value;
        } else if (strcmp(name, "PageTables:") == 0) {
            mi->pagetables = value;
        }
    }
    fclose(fp);

    return 0;
}

int MachineStatus::get_systemNetwork(Network* nt)
{
    FILE *fp = NULL;

    char buf[1024];
    char name[32];
    long long ibyte, ipacket, ierr, obyte, opacket, oerr, tmp;

    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
        return -1;

    //first skip two line(titles)
    fgets(buf, 1024, fp);
    fgets(buf, 1024, fp);

    nt->ibytes = 0;
    nt->ipackets = 0;
    nt->obytes = 0;
    nt->opackets = 0;
    nt->ierrs = 0;
    nt->oerrs = 0;
    //face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    while(fgets(buf, 1024, fp) != NULL) {
        name[0] = '\0';
        sscanf(buf, "%s:", name);
        if (strcmp(name, "lo:") == 0){
            memset(buf, 0, sizeof(buf));
            continue;
        }

        sscanf(buf, "%s%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld",
                name, &ibyte, &ipacket, &ierr, &tmp, &tmp, &tmp, &tmp, &tmp, &obyte, &opacket, &oerr);

        nt->ibytes += (double)ibyte;
        nt->ipackets += (double)ipacket;
        nt->ierrs += (double)ierr;
        nt->obytes += (double)obyte;
        nt->opackets += (double)opacket;
        nt->oerrs += (double)oerr;

        memset(buf, 0, sizeof(buf));
    }
    fclose(fp);

    return 0;
}

int MachineStatus::get_systemFileNR(FileNR *fnr)
{
    unsigned int tmp;
    FILE *file = NULL;

    file = fopen("/proc/sys/fs/file-nr", "r");
    if (file == NULL) {
        return -1;
    }

    fscanf(file, "%d %d %d", &fnr->nropen, &tmp, &fnr->nrtotal);
    fclose(file);

    return 0;
}

int MachineStatus::get_systemSocketStats(SocketStats* ss)
{
    FILE *file = NULL;
    file = fopen("/proc/net/sockstat", "r");
    if (file == NULL)
        return -1;

    char buf[1024];
    char name[128];
    int tmp;
    while(fgets(buf, 1024, file) != NULL) {
        name[0] = '\0';
        sscanf(buf, "%s", name);
        if (strcmp(name, "TCP:") == 0) {
            sscanf(buf, "%s %s %d %s %d %s %d %s %d", name, name,
                    &ss->tcpuse, name, &tmp, name, &ss->tcpwait, name, &ss->tcpalloc);
        } else if (strcmp(name, "UDP:") == 0) {
            sscanf(buf, "%s %s %d", name, name, &ss->udpuse);
        }
    }
    fclose(file);

    return 0;
}

int MachineStatus::get_systemSnmpTcpProtocol(SnmpTcpProtocol* stp)
{
    FILE *file = NULL;
    file = fopen("/proc/net/snmp", "r");
    if (file == NULL)
        return -1;

    char buf[1024];
    int is_title = 0;
    char name[128];
    while(fgets(buf, 1024, file) != NULL) {
        name[0] = '\0';
        sscanf(buf, "%s", name);
        if (strcmp(name, "Tcp:")  == 0) {
            if (is_title == 0) {
                is_title = 1;
                continue;
            }
            int tmp;
            sscanf(buf, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d", name, &tmp, &tmp, &tmp, &tmp,
                    &stp->activeopen, &stp->pasiveopen, &stp->afails, &stp->eresets,
                    &tmp, &stp->insegs, &stp->outsegs, &tmp, &stp->inerrs, &stp->outrst);
        }
    }
    fclose(file);
    return 0;
}

int MachineStatus::get_processName(char*pn) {
#ifdef linux
	char path[1024];
    int pathLen = 0;
    if ((pathLen = readlink("/proc/self/exe", path, 1024)) <= 0) {
        return -1;
    }
    path[pathLen] = '\0';
    char* name = NULL;
    name = strrchr(path, '/');
    if (name == NULL)
        return -1;
    ++name;
    strcpy(pn, name);
#endif
    return 0;
}

int MachineStatus::get_processPid(char* pn) {
    FILE *file = NULL;
    int pid = 0;
    char command[1024];

    if (pn == NULL || strlen(pn) <= 0)
        return -1;

    memset(command, 0, 1024);
    sprintf(command, "pidof %s", pn);
    if ((file = popen(command, "r")) == NULL){
        return -1;
    }

    fscanf(file, "%d", &pid);
    pclose(file);

    return pid;
}

int MachineStatus::get_systemCurrentProcessInfo(CurrentProcessInfo& cpi)
{
    char pn[1024];
    int pid = 0;
    char command[1024];
    FILE *file = NULL;
    char buf[2048];

    memset(pn, 0, 1024);
    get_processName(pn);
    pid = get_processPid(pn);
    if (pid <= 0)
        return -1;

    memset(command, 0, 1024);
    sprintf(command, "top -p %d -bn 1 | awk 'NR==8{print}'", pid);
    if ((file = popen(command, "r")) == NULL) {
        return -1;
    }

    memset(buf, 0, 2048);
    fgets(buf, 2048, file);
    pclose(file);
    memset(cpi.user, 0, sizeof(cpi.user));
    memset(cpi.command, 0, sizeof(cpi.command));
    memset(cpi.status, 0, sizeof(cpi.status));
    memset(cpi.time, 0, sizeof(cpi.time));

    sscanf(buf, "%d %s %d %d %s %s %s %s %lf %lf %s %s",
            &cpi.pid, cpi.user, &cpi.pr, &cpi.ni, cpi.virt, cpi.res,
            cpi.shr, cpi.status, &cpi.cpu, &cpi.mem, cpi.time, cpi.command);
    return 0;
}

int MachineStatus::get_systemInfo(OSLoadData* osd)
{
    get_systemLoadAverage(&osd->la);
    get_systemCpuInfo(&osd->ci);
    get_systemFileNR(&osd->fnr);
    get_systemMemoryInfo(&osd->mi);
    get_systemNetwork(&osd->nt);
    get_systemPageSwap(&osd->ps);
    get_systemSnmpTcpProtocol(&osd->stp);
    get_systemSocketStats(&osd->ss);
    return 0;
}

int MachineStatus::intDouble(double a, double b)
{
   int rtnval = 0;
   if (b - a > 0.0000001)
   {
       rtnval = ((int)(b-a)) & 0x8fffffff;
   }
   else
   {
       rtnval = ((int)(b + 4294967295.0 - a)) & 0x8fffffff;
   }
   if (rtnval < 0) rtnval = 0;
   return rtnval;
}

double MachineStatus::doubleDouble(double a, double b)
{
   if (b-a > 0.0000001)
   {
       return (b - a);
   }
   return (b + 4294967295.0 - a);
}

void MachineStatus::getOSLoadRecord(OSLoadData *os1, OSLoadData *os2, OSLoadRecord *rec)
{
     float duser, dsystem, diowait, didle, dtotal, dirq, dsoftirq;

     duser = os2->ci.user - os1->ci.user;
     dsystem = os2->ci.system - os1->ci.system;
     diowait = os2->ci.iowait - os1->ci.iowait;
     didle   = os2->ci.idle - os1->ci.idle;
     dirq     = os2->ci.irq  - os1->ci.irq;
     dsoftirq = os2->ci.softirq - os1->ci.softirq;
     dtotal  = duser + dsystem + diowait + didle + dirq + dsoftirq ;
     if (fabs(dtotal) < 0.0000001) {
    	 return;
     }

     rec->load = os2->la.load1m;
     rec->run  = os2->la.procrun;
     rec->num  = os2->la.procnum;

     rec->user = (int)(100 * duser / dtotal);
     rec->system = (int)(100 * dsystem / dtotal);
     rec->iowait = (int)(100 * diowait / dtotal);
     rec->irq    = (int)(100 * dirq / dtotal);
     rec->softirq= (int)(100 * dsoftirq / dtotal);

     rec->irqcall=intDouble(os1->ci.irqcall , os2->ci.irqcall);

     rec->context= os2->ci.context - os1->ci.context;
     rec->fork   = os2->ci.processes - os1->ci.processes;
     rec->uptime = (time(0) - os2->ci.uptime)/3600;

     rec->pgin = os2->ps.pgpgin - os1->ps.pgpgin;
     rec->pgout = os2->ps.pgpgout - os1->ps.pgpgout;
     rec->swpin = os2->ps.pswpin - os1->ps.pswpin;
     rec->swpout = os2->ps.pswpout - os1->ps.pswpout;

     rec->free = os2->mi.memfree;
     rec->swap = os2->mi.swaptotal - os2->mi.swapfree;
     rec->lfree = os2->mi.lowfree;
     rec->cache = os2->mi.cached;
     rec->hfree = os2->mi.highfree;
     rec->pagetables = os2->mi.pagetables;

     rec->ibytes = intDouble(os1->nt.ibytes , os2->nt.ibytes);
     rec->ipackets = intDouble(os1->nt.ipackets , os2->nt.ipackets);
     rec->ierrors = (int)(os2->nt.ierrs - os1->nt.ierrs);
     rec->obytes = intDouble(os1->nt.obytes , os2->nt.obytes);
     rec->opackets = intDouble(os1->nt.opackets , os2->nt.opackets);
     rec->oerrors = (int)(os2->nt.oerrs - os1->nt.oerrs);

     if (os2->fnr.nrtotal != 0) {
    	 rec->nr = 100 * os2->fnr.nropen / os2->fnr.nrtotal;
     }
     rec->nropen = os2->fnr.nropen;
     rec->nrtotal = os2->fnr.nrtotal;

     rec->tcpuse = os2->ss.tcpuse;
     rec->tcpalloc = os2->ss.tcpalloc;
     rec->tcpwait = os2->ss.tcpwait;
     rec->aopen = os2->stp.activeopen - os1->stp.activeopen;
     rec->popen = os2->stp.pasiveopen - os1->stp.pasiveopen;

     rec->afail = os2->stp.afails- os1->stp.afails;
     rec->reset = os2->stp.eresets- os1->stp.eresets;
     rec->isegs = os2->stp.insegs- os1->stp.insegs;
     rec->osegs = os2->stp.outsegs- os1->stp.outsegs;
     rec->rsegs = os2->stp.resegs- os1->stp.resegs;
     rec->inerr = os2->stp.inerrs- os1->stp.inerrs;
     rec->orest = os2->stp.outrst- os1->stp.outrst;
}
