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
 * @FileName: readwritelock.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年12月9日 上午10:01:19
 *  
 */

#include "readwritelock.h"
#include "logger.h"

ReadWriteLock::ReadWriteLock() {
	// TODO Auto-generated constructor stub
	init_lock();
}

ReadWriteLock::~ReadWriteLock() {
	// TODO Auto-generated destructor stub
#ifndef WIN32
	pthread_rwlock_destroy(&lock);
#endif
}

int ReadWriteLock::init_lock()
{
#ifndef WIN32
	int res = pthread_rwlock_init(&lock, NULL);
	if (res != 0) {
		logs(Logger::FATAL, "init rwlock error");
		return res;
	}
#endif
	return 0;
}

void ReadWriteLock::read_lock()
{
#ifndef WIN32
	pthread_rwlock_rdlock(&lock);
#else
	lock.lock();
#endif
}

void ReadWriteLock::read_unlock()
{
#ifndef WIN32
	pthread_rwlock_unlock(&lock);
#else
	lock.unlock();
#endif
}

void ReadWriteLock::write_lock()
{
#ifndef WIN32
	pthread_rwlock_wrlock(&lock);
#else
	lock.lock();
#endif
}

void ReadWriteLock::write_unlock()
{
	this->read_unlock();
}
