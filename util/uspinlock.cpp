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
 * @FileName: spinlock.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年12月9日 上午10:27:45
 *  
 */

#include "uspinlock.h"

USpinLock::USpinLock() {
	// TODO Auto-generated constructor stub
#ifndef WIN32
	pthread_spin_init(&m_lock, 0);
#endif
}

USpinLock::~USpinLock() {
	// TODO Auto-generated destructor stub
#ifndef WIN32
	pthread_spin_destroy(&m_lock);
#endif
}

void USpinLock::lock() {
#ifndef WIN32
	pthread_spin_lock(&m_lock);
#else
	m_lock.lock();
#endif
}

void USpinLock::unlock() {
#ifndef WIN32
	pthread_spin_unlock(&m_lock);
#else
	m_lock.unlock();
#endif
}
