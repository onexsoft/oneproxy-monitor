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
 * @FileName: assistthread.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年11月9日 下午1:38:52
 *  
 */

#ifndef ASSISTTHREAD_H_
#define ASSISTTHREAD_H_

#include "thread.h"
#include "connectionpool.h"

class ConnectManager;
class AssistThread : public Thread{
private:
	ConnectManager* manager;
	bool is_stop;
public:
	AssistThread(ConnectManager* manager);
	~AssistThread();

	void stop();
	bool get_stop();
private:
	static thread_start_func(start);

};

#endif /* ASSISTTHREAD_H_ */
