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
 * @FileName: parsedata.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月16日 下午4:16:08
 *  
 */

#ifndef PARSEDATA_H_
#define PARSEDATA_H_

#include "thread.h"
#include "mutexlock.h"
#include "monitor_define.h"

typedef struct _handle_func_param_t{
	MonitorFunc func;
	void* func_args;
	std::string className;// className and func is exclusion.
}FuncParam;
typedef std::map<int, FuncParam> IntFuncParamMap;
typedef std::map<std::string, FuncParam> StringFuncParamMap;

class ParseData : public Thread{
public:
	ParseData();
	virtual ~ParseData();
	void regester_protocol();
	void add_taskData(TaskDataT* taskData);
	void add_portHandle(int port, std::string className, MonitorFunc func, void* func_args);
	void add_nameHandle(std::string name, std::string className, MonitorFunc func, void* func_args);
	void set_stop();
private:
	static thread_start_func(start);
	TaskDataT* get_taskData();
	void handle_taskData(TaskDataT* taskData);
	void handle_protocolData(TaskDataT* taskData);
	void* get_protocolBase(TaskDataT* taskData);
	Connection* new_connection(TaskDataT* taskData);
	void free_connection(Connection* conn);
private:
	declare_class_member(MutexLock, lock);
	declare_class_member(TaskDataList, taskDataList);
	declare_class_member(bool, isStop)
	IntFuncParamMap portHandleMap;
	StringFuncParamMap nameHandleMap;
	TaskDataKeyConnMap taskDataConnMap;//use in monitor
};

#endif /* PARSEDATA_H_ */
