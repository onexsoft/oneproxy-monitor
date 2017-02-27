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
 * @FileName: managerbase.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月23日 下午2:50:22
 *  
 */

#ifndef MANAGERBASE_H_
#define MANAGERBASE_H_

#include "vip.h"
#include "httpserver.h"
#include "assistthread.h"
#include "define.h"

typedef void (*SignalHandleFunc)(int);
class ManagerBase {
public:
	ManagerBase();
	virtual ~ManagerBase();
	void start();
protected:
	virtual void start_child() = 0;
	void set_signalHandleFunc(SignalHandleFunc func);
private:
	Vip vipThread;
	HttpServer httpServer;
	AssistThread assistThread;
	SignalHandleFunc func;
};

#endif /* MANAGERBASE_H_ */
