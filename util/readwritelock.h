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
 * @FileName: readwritelock.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年12月9日 上午10:01:19
 *  
 */

#ifndef UTIL_READWRITELOCK_H_
#define UTIL_READWRITELOCK_H_

#include <pthread.h>
#include "mutexlock.h"

class ReadWriteLock {
public:
	ReadWriteLock();
	virtual ~ReadWriteLock();

	int init_lock();

	void read_lock();
	void write_lock();

	void read_unlock();
	void write_unlock();
private:
#ifdef WIN32
	MutexLock lock;
#else
	pthread_rwlock_t lock;
#endif
};

#endif /* UTIL_READWRITELOCK_H_ */
