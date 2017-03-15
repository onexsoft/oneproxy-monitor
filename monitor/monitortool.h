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
 * @FileName: monitortool.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年3月15日 上午11:45:01
 *  
 */

#ifndef MONITOR_MONITORTOOL_H_
#define MONITOR_MONITORTOOL_H_

#include <dirent.h>
#include <limits.h>

#include "define.h"
#include <string>
#include <iostream>
#include "monitor_define.h"

class MonitorTool {
public:
	MonitorTool();
	virtual ~MonitorTool();
	std::string get_processName(int port, bool isTcp, bool reload);
private:
	//read progress name and port relation.
	void load_progress_info();
#ifdef linux
	void extract_type_1_socket_inode(const char lname[], long * inode_p);
	void extract_type_2_socket_inode(const char lname[], long * inode_p);
#endif

	void linux_load_progress_info_bylsof();
	int get_inodeBasePort(unsigned short int port, unsigned long& inode, bool isTcp);
	int get_inodeBasePort(unsigned short int port, unsigned long& inode, char* fileName);
private:
	IntStringMap portNameMap;//port and progress name map.
	ULongStringMap nodeNameMap;//node and progress name map.
};

#endif /* MONITOR_MONITORTOOL_H_ */
