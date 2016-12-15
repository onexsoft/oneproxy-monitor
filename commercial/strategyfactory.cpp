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
 * @FileName: strategyfactory.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年12月7日 下午5:17:10
 *  
 */

#include "strategyfactory.h"
#ifndef GITHUB_OPENSOURCE
#include "commericialimpl.h"
#endif
#include "mutexlock.h"

FreeImpl* StrategyFactory::impl = NULL;
MutexLock StrategyFactory::mutexLock;
FreeImpl* StrategyFactory::get_strategy()
{
	if (impl == NULL) {
		mutexLock.lock();
		if (impl == NULL) {
#ifdef GITHUB_OPENSOURCE
			impl = new FreeImpl();
#else
			if (userIsFree()) {
				impl = new FreeImpl();
			} else {
				impl = new CommericialImpl();
			}
#endif
		}
		mutexLock.unlock();
	}
	return impl;
}

StrategyFactory::~StrategyFactory() {
	if (StrategyFactory::impl != NULL) {
		delete StrategyFactory::impl;
		StrategyFactory::impl = NULL;
	}
}

bool StrategyFactory::userIsFree() {
	return false;
}
