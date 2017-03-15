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
 * @FileName: monitortool.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年3月15日 上午11:45:01
 *  
 */

#include "monitortool.h"
#include "logger.h"
#include "monitor_define.h"

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

MonitorTool::MonitorTool() {
	// TODO Auto-generated constructor stub
	this->load_progress_info();
}

MonitorTool::~MonitorTool() {
	// TODO Auto-generated destructor stub
}

void MonitorTool::load_progress_info() {
#ifdef linux
	DIR *dirproc = opendir(PATH_PROC);
	if (dirproc == NULL) {
		logs(Logger::ERR, "open proc directory error, please use root user");
		return;
	}
	this->nodeNameMap.clear();

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
				this->nodeNameMap[(unsigned long)inode] = std::string(cmdlp);
			}
		}
		closedir(dirfd);
		dirfd = NULL;
	}
	closedir(dirproc);
	dirproc = NULL;
#else
	FILE* pPipe = NULL;
	const int bufferSize = 256;
	char psBuffer[bufferSize];
	char name[1024];
	int pid;
	char ip[32];
	char tmp[64];
	int port = 0;

	this->portNameMap.clear();
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
void MonitorTool::extract_type_1_socket_inode(const char lname[], long * inode_p) {

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

void MonitorTool::extract_type_2_socket_inode(const char lname[], long * inode_p) {

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

std::string MonitorTool::get_processName(int port, bool isTcp, bool reload) {
#ifdef _WIN32
IntStringMap& tmpMap = this->portNameMap;
#else
ULongStringMap& tmpMap = this->nodeNameMap;
#endif

#ifndef _WIN32
	unsigned long tport = 0;
	if (this->get_inodeBasePort((unsigned short int)port, tport, isTcp)) {
		logs(Logger::ERR, "get inode base port error");
		return std::string();
	}
	ULongStringMap::iterator it = tmpMap.find(tport);
#else
	int tport = port;
	IntStringMap::iterator it = tmpMap.find(tport);
#endif

	if (it == tmpMap.end() && reload) {
		if (tport <= 0) {
			return std::string();
		}
		//reload
		this->load_progress_info();
		if (it == tmpMap.end())
			it = tmpMap.find(tport);
	}

	if (it == tmpMap.end()) {
		return std::string();
	}
	return it->second;
}

void MonitorTool::linux_load_progress_info_bylsof() {
	//lsof -Pnl +M  -i4 -i6 -F cn
	std::string command = std::string("lsof -Pnl +M -i4 -i6 -F cn");
	FILE* file = popen(command.c_str(), "r");
	if (file == NULL) {
		logs(Logger::ERR, "popen error(%s)", SystemApi::system_strerror());
		return;
	}
	int port = 0;
	std::string name;
	const int BUFSIZE = 1024;
	char buf[BUFSIZE];
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

int MonitorTool::get_inodeBasePort(unsigned short int port, unsigned long& inode, bool isTcp)
{
	inode = 0;
	if (isTcp == false) {
		char* file = (char*)"/proc/net/udp";
		this->get_inodeBasePort(port, inode, file);
		if (inode == 0) {
			file = (char*)"/proc/net/udp6";
			this->get_inodeBasePort(port, inode, file);
		}
	} else {
		char* file = (char*)"/proc/net/tcp";
		this->get_inodeBasePort(port, inode, file);
		if (inode == 0) {
			file = (char*)"/proc/net/tcp6";
			this->get_inodeBasePort(port, inode, file);
		}
	}

	return 0;
}

int MonitorTool::get_inodeBasePort(unsigned short int port, unsigned long& inode, char* fileName)
{
	FILE* fd = fopen(fileName, "r");
	if (fd == NULL) return -1;

	char line[LINE_MAX] = {0};
	fgets(line, sizeof(line), fd);//skip title

	char local_addr[64], rem_addr[64];
	char more[512];
	int num, local_port, rem_port, d, state, timer_run, uid, timeout;
	unsigned long rxq, txq, time_len, retr, tinode;
	while(fgets(line, sizeof(line), fd)) {
		num = sscanf(line,
				 "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %512s\n",
				 &d, local_addr, &local_port,
				 rem_addr, &rem_port, &state,
			  &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &tinode, more);

		if ((int)port == local_port) {
			inode = tinode;
			break;
		}
	}
	fclose(fd);
	return 0;
}
