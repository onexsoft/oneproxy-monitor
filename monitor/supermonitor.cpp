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
#include <iostream>
#include <string>
#include <iostream>
#include <dirent.h>
#include <limits.h>

#ifdef linux
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif

#ifndef LINE_MAX
#define LINE_MAX 4096
#endif
#define PATH_PROC	   "/proc"
#define PATH_FD_SUFF	"fd"
#define PATH_FD_SUFFl       strlen(PATH_FD_SUFF)
#define PATH_PROC_X_FD      PATH_PROC "/%s/" PATH_FD_SUFF
#define PATH_CMDLINE	"cmdline"
#define PATH_CMDLINEl       strlen(PATH_CMDLINE)
#define PROGNAME_WIDTH 20

#define PRG_INODE	 "inode"
#define PRG_SOCKET_PFX    "socket:["
#define PRG_SOCKET_PFXl (strlen(PRG_SOCKET_PFX))
#define PRG_SOCKET_PFX2   "[0000]:"
#define PRG_SOCKET_PFX2l  (strlen(PRG_SOCKET_PFX2))

SuperMonitor::SuperMonitor(int workThreadNum)
	:Thread(thread_type_manager, std::string("super_monitor")){
	// TODO Auto-generated constructor stub

	this->m_captureData = NULL;
	this->m_stop = false;

	for (int i = 0; i < workThreadNum; ++i) {
		ParseData* pd = new ParseData();
		if (pd != NULL) {
			this->m_parseDataVec.push_back(pd);
		}
	}

	this->startThread(SuperMonitor::start, this);
}

SuperMonitor::~SuperMonitor() {
	// TODO Auto-generated destructor stub
	this->joinThread();

	std::vector<ParseData*>::iterator it = this->m_parseDataVec.begin();
	for(; it != this->m_parseDataVec.end();) {
		ParseData* pd = (*it);
		++it;
		if (pd) {
			pd->joinThread();
			delete pd;
			pd = NULL;
		}
	}

	while(this->taskList.size() > 0) {
		TaskT* tt = this->get_taskFromList();
		if (tt) {
			delete tt;
			tt = NULL;
		}
	}
}

void SuperMonitor::set_stop() {
	std::vector<ParseData*>::iterator it = this->m_parseDataVec.begin();
	for(; it != this->m_parseDataVec.end(); ++it) {
		(*it)->set_stop();
	}
	this->m_stop = true;
	this->taskLock.lock();
	this->taskLock.signal_mutexCond();
	this->taskLock.unlock();
}

void SuperMonitor::add_desPort(int port) {
	this->m_desPortMap[port] = 1;
}

void SuperMonitor::del_desPort(int port) {
	this->m_desPortMap.erase(port);
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
	while(sm->get_stop() == false || sm->taskList.size() > 0) {
		if (sm->taskList.size() > 0) {
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
	if (this->taskList.size() <= 0)
		return NULL;

	TaskT* t = NULL;
	this->taskLock.lock();
	if (this->taskList.size() > 0) {
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

	static std::string localIp = this->m_captureData->get_localIp();
	u_char* packet = (u_char*)task->packet->addr();
	struct pcap_pkthdr* pkthdr = &task->pkthdr;
	struct ether_header* eptr = (struct ether_header*)packet;
	int ether_type = ntohs(eptr->ether_type);
	const unsigned char* payload = packet + sizeof(struct ether_header);

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

	if (!this->m_desPortMap.empty())
	{
		do{
			if (localIp.empty() && (m_desPortMap.find(srcPort) != m_desPortMap.end()
					|| m_desPortMap.find(desPort) != m_desPortMap.end()))
				break;

			if (localIp == srcIp && localIp == desIp
					&& (m_desPortMap.find(srcPort) != m_desPortMap.end()
							|| m_desPortMap.find(desPort) != m_desPortMap.end())) {
				break;//target port data.
			}

			if (localIp == srcIp && this->m_desPortMap.find(srcPort) != this->m_desPortMap.end()){
				break;// target port data.
			}

			if (localIp == desIp && this->m_desPortMap.find(desPort) != this->m_desPortMap.end()) {
				break; // target port data.
			}

			free_task(); // is not target data.
			return;
		} while(0);
	}

	bool reload_processInfo = false;
	int thl = 0;
	u_char* data = NULL;
	ConnFlag flag = CONN_INIT;
	if (iptr->ip_p == IPPROTO_TCP) {
		reload_processInfo = true;
		thl = TH_OFF(tcpHeader) * 4;
		flag = CONN_TCP;
		if (tcpHeader->th_flags & TH_SYN) {
			flag = CONN_TCP_SYN;
		} else if (tcpHeader->th_flags & TH_FIN){
			flag = CONN_TCP_FIN;
		}
	} else if (iptr->ip_p == IPPROTO_UDP) {
		thl = 8;
		flag = CONN_UDP;
	}
	data = (u_char*)((char*)tcpHeader + thl);
	int ipLen = ntohs(iptr->ip_len);
	int ipHeaderLen = IP_HL(iptr) * 4;
	int dataLen = ipLen - ipHeaderLen - thl;

	if (dataLen <= 0 && (flag == CONN_INIT || flag == CONN_UDP)) {
		free_task();
		return ;
	}
	dataLen = pkthdr->len - (int)(data - packet);
	if ((data == NULL || dataLen <= 0) && (flag == CONN_INIT || flag == CONN_UDP)){
		free_task();
		return;
	}

	if (this->m_parseDataVec.size() > 0) {
		TaskDataT *tdt = new TaskDataT();
		tdt->ts = pkthdr->ts;
		tdt->flag = flag;
		tdt->key.desIp = desIp;
		tdt->key.srcIp = srcIp;
		tdt->key.desPort = desPort;
		tdt->key.srcPort = srcPort;
		tdt->key.gen_cmpKey();
		tdt->data.append(data, dataLen);
		static int vs = this->m_parseDataVec.size();
		if (localIp.empty()) {
			tdt->pi.port = 0;
			tdt->pi.name.clear();
			this->m_parseDataVec.at(0)->add_taskData(tdt);
		} else if (localIp == srcIp) {//send
			tdt->pi.port = srcPort;
			tdt->pi.name = this->get_processName(tdt->pi.port, reload_processInfo);
			this->m_parseDataVec.at(desPort % vs)->add_taskData(tdt);
		} else {//recv
			tdt->pi.port = desPort;
			tdt->pi.name = this->get_processName(tdt->pi.port, reload_processInfo);
			this->m_parseDataVec.at(srcPort % vs)->add_taskData(tdt);
		}
	}

	free_task();
#undef free_task
}

void SuperMonitor::load_progress_info() {
#ifdef linux
#if 0
	DIR *dirproc = opendir(PATH_PROC);
	if (dirproc == NULL) {
		logs(Logger::ERR, "open proc directory error, please use root user");
		return;
	}
	this->portNameMap.clear();

	char lname[30];
	char line[LINE_MAX];
	struct dirent *direproc = NULL;
	struct dirent *direfd = NULL;
	while((direproc = readdir(dirproc))) {
		char* cs = NULL;
		for (cs = direproc->d_name; *cs; cs++)//判断是否全部为数字
			if (!isdigit(*cs))
				break;
		if (*cs)//说明名称中有非数字
			continue;

		int procfdlen = snprintf(line, sizeof(line), PATH_PROC_X_FD, direproc->d_name);
		if (procfdlen <= 0 || procfdlen >= (int)(sizeof(line) - 5))
			continue;

		errno = 0;
		DIR* dirfd = opendir(line);//open fd directory
		if (!dirfd) {
			if (errno == EACCES)
				logs(Logger::ERR, "can't access %s", line);
			continue;
		}

		line[procfdlen] = '/';
		const char *cmdlp = NULL;
		while((direfd = readdir(dirfd))) {
			if (procfdlen + 1 + strlen(direfd->d_name) + 1 > sizeof(line))
				continue;
			memcpy(line + procfdlen - PATH_FD_SUFFl, PATH_FD_SUFF "/", PATH_FD_SUFFl + 1);
			strcpy(line + procfdlen + 1, direfd->d_name);
			int lnamelen = readlink(line, lname, sizeof(lname) - 1);
			lname[lnamelen] = '\0';  /*make it a null-terminated string*/

			long inode = 0;
			extract_type_1_socket_inode(lname, &inode);
			if (inode < 0) extract_type_2_socket_inode(lname, &inode);
			if (inode < 0) continue;

			if (cmdlp == NULL) {
				if (procfdlen - PATH_FD_SUFFl + PATH_CMDLINEl >= sizeof(line) - 5)
					continue;
				strcpy(line + procfdlen - PATH_FD_SUFFl, PATH_CMDLINE);
				int fd = open(line, O_RDONLY);
				if (fd < 0) continue;

				char cmdlbuf[512];
				int cmdllen = read(fd, cmdlbuf, sizeof(cmdlbuf) - 1);
				if (close(fd)) continue;

				if (cmdllen == -1) continue;
				if (cmdllen < (int)(sizeof(cmdlbuf) - 1))
					cmdlbuf[cmdllen]='\0';

				if ((cmdlp = strrchr(cmdlbuf, '/')))
					cmdlp++;
				else
					cmdlp = cmdlbuf;
			}

			char finbuf[PROGNAME_WIDTH];
			snprintf(finbuf, sizeof(finbuf), "%s/%s", direproc->d_name, cmdlp);
			if (strlen(cmdlp) > 0) {
				this->portNameMap[inode] = std::string(cmdlp);
			}
		}
		closedir(dirfd);
		dirfd = NULL;
	}
	closedir(dirproc);
	dirproc = NULL;

#else
	linux_load_progress_info_bylsof();
#endif
#else
	FILE* pPipe = NULL;
	const int bufferSize = 256;
	char psBuffer[bufferSize];
	char name[1024];
	int pid;
	char ip[32];
	char tmp[64];
	int port = 0;

	std::map<int, std::string> pidNameMap;
	if ((pPipe = _popen("tasklist", "rt")) == NULL) {
		logs(Logger::ERR, "open tasklist error");
		return;
	}
	while(fgets(psBuffer, bufferSize, pPipe)) {
		sscanf(psBuffer, "%s %d", name, &pid);
		if (pid > 0) {
			pidNameMap[pid] = std::string(name);
		}
	}
	_pclose(pPipe);

	if ((pPipe = _popen("netstat -ano", "rt")) == NULL) {
		logs(Logger::ERR, "popen netstat -ano error");
		return ;
	}
	while(fgets(psBuffer, bufferSize, pPipe)) {
		if (strstr(psBuffer, "TCP") == NULL && strstr(psBuffer, "UDP") == NULL) {
			continue;
		}
		sscanf(psBuffer, "%s %s %s %s %d", tmp, ip, tmp, tmp, &pid);
		int ipLen = strlen(ip);
		int i = ipLen - 1;
		for(; i >= 0; --i) {
			if (ip[i] == ':')
				break;
		}
		i = i + 1;
		if (i > 0 && i < ipLen - 1) {
			port = atoi((char*)(ip + i));
		}
		std::string name = pidNameMap[pid];
		if (!name.empty() && port > 0) {
			this->portNameMap[port] = name;
		}
	}
	_pclose(pPipe);
#endif
}

#ifdef linux
void SuperMonitor::extract_type_1_socket_inode(const char lname[], long * inode_p) {

    /* If lname is of the form "socket:[12345]", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFXl+3) *inode_p = -1;
    else if (memcmp(lname, PRG_SOCKET_PFX, PRG_SOCKET_PFXl)) *inode_p = -1;
    else if (lname[strlen(lname)-1] != ']') *inode_p = -1;
    else {
        char inode_str[strlen(lname + 1)];  /* e.g. "12345" */
        const int inode_str_len = strlen(lname) - PRG_SOCKET_PFXl - 1;
        char *serr;

        strncpy(inode_str, lname+PRG_SOCKET_PFXl, inode_str_len);
        inode_str[inode_str_len] = '\0';
        *inode_p = strtol(inode_str,&serr,0);
        if (!serr || *serr || *inode_p < 0 || *inode_p >= INT_MAX)
            *inode_p = -1;
    }
}

void SuperMonitor::extract_type_2_socket_inode(const char lname[], long * inode_p) {

    /* If lname is of the form "[0000]:12345", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */
    if (strlen(lname) < PRG_SOCKET_PFX2l+1) *inode_p = -1;
    else if (memcmp(lname, PRG_SOCKET_PFX2, PRG_SOCKET_PFX2l)) *inode_p = -1;
    else {
        char *serr;

        *inode_p=strtol(lname + PRG_SOCKET_PFX2l,&serr,0);
        if (!serr || *serr || *inode_p < 0 || *inode_p >= INT_MAX)
            *inode_p = -1;
    }
}
#endif

std::string SuperMonitor::get_processName(int port, bool reload) {
	IntStringMap::iterator it = this->portNameMap.find(port);
	if (it == this->portNameMap.end() && reload) {
		//reload
		this->load_progress_info();
		it = this->portNameMap.find(port);
	}

	if (it == this->portNameMap.end()) {
		return std::string();
	}
	return it->second;
}

void SuperMonitor::linux_load_progress_info_bylsof() {
	//lsof -Pnl +M  -i4 -i6 -F cn
	std::string command = std::string("lsof -Pnl +M -i4 -i6 -F cn");
	FILE* file = popen(command.c_str(), "r");
	if (file == NULL) {
		logs(Logger::ERR, "popen error(%s)", SystemApi::system_strerror());
		return;
	}

	char buf[1024] = {0};
	int port = 0;
	std::string name;
	while(fgets(buf, sizeof(buf), file) != NULL) {
		if (buf[0] == 'p') {
			port = 0;
			name.clear();
			continue;
		} else if (buf[0] == 'c') {
			name = std::string(buf + 1, strlen(buf) - 2);
		} else if (buf[0] == 'n') {
			port = 0;
			std::string portString = std::string(buf + 1, strlen(buf) - 1);
			size_t endPos = portString.find("->", 0);
			if (endPos < portString.length()) {
				portString = portString.substr(0, endPos);
			}

			endPos = portString.find_last_of(':');
			if (endPos != portString.npos) {
				port = atoi(portString.substr(endPos + 1, portString.length() - endPos - 1).c_str());
			}

			if (port > 0 && !name.empty()) {
				//std::cout << "port: " << port << " name: " << name << std::endl;
				this->portNameMap[port] = name;
			}
		}
	}

	pclose(file);

}
