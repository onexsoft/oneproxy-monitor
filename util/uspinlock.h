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
 * @FileName: spinlock.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年12月9日 上午10:27:45
 *  
 */

#ifndef UTIL_USPINLOCK_H_
#define UTIL_USPINLOCK_H_
#include "mutexlock.h"
#ifndef WIN32
#include <bits/pthreadtypes.h>
#endif
#include <pthread.h>

#ifdef WIN32
typedef MutexLock LockHandle;
#else
typedef pthread_spinlock_t LockHandle;
#endif

class USpinLock {
public:
	USpinLock();
	virtual ~USpinLock();
	void lock();
	void unlock();
private:
	LockHandle m_lock;
};

#endif /* UTIL_USPINLOCK_H_ */
