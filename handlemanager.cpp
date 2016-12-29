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
 * @FileName: handlemanager.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年10月27日 上午9:49:30
 *  
 */

#include "logger.h"
#include "handlemanager.h"

HandleManager::HandleManager()
{
	this->lastAllocHandle = 1;
}

HandleManager::~HandleManager()
{
	this->frontBackendHandleMap.clear();
	this->globalHandleUseFlag.clear();
}

int HandleManager::add_handle(unsigned int backendPreparedHandle, unsigned int backendCursorHandle,
		unsigned int hashCode, void* servns, FrontHandle& frontHandle)
{
	if (backendCursorHandle) {
		uif (this->get_globalHandleId(frontHandle.cursorHandle)) {
			logs(Logger::ERR, "get front cursor handle error");
			return -1;
		}
	}

	if (backendPreparedHandle) {
		uif (this->get_globalHandleId(frontHandle.preparedHandle)) {
			logs(Logger::ERR, "get front prepared handle error");
			return -1;
		}
	}

	logs(Logger::DEBUG, "preparedHandle: %d, cursorHandle: %d",
			frontHandle.preparedHandle, frontHandle.cursorHandle);

	BackendHandle backendHandle;
	backendHandle.handle.cursorHandle = backendCursorHandle;
	backendHandle.handle.preparedHandle = backendPreparedHandle;
	backendHandle.hashCode = hashCode;
	backendHandle.pointer = servns;

	this->frontBackendHandleMap[frontHandle] = backendHandle;
	return 0;
}

int HandleManager::add_handle(const BackendHandle& backendHandle, FrontHandle& frontHandle, bool useGlobalId)
{
	if (useGlobalId) {
		if (backendHandle.handle.cursorHandle) {
			uif (this->get_globalHandleId(frontHandle.cursorHandle)) {
				logs(Logger::ERR, "get front cursor handle error");
				return -1;
			}
		}

		if (backendHandle.handle.preparedHandle) {
			uif (this->get_globalHandleId(frontHandle.preparedHandle)) {
				logs(Logger::ERR, "get front prepared handle error");
				return -1;
			}
		}
	}

	logs(Logger::DEBUG, "preparedHandle: %u, cursorHandle: %u",
			frontHandle.preparedHandle, frontHandle.cursorHandle);
	this->frontBackendHandleMap[frontHandle] = backendHandle;

//	this->show_fbHandleMap();
	return 0;
}

int HandleManager::get_backendHandle(FrontHandle frontHandle, BackendHandle& backendHandle)
{
	//this->show_fbHandleMap();
	logs(Logger::DEBUG, "preaparedHandle: %u, cursorHandle: %u",
			frontHandle.preparedHandle, frontHandle.cursorHandle);
	FBHandleMap::iterator it = this->frontBackendHandleMap.find(frontHandle);
	if (it != this->frontBackendHandleMap.end()) {
		backendHandle = it->second;
		return 0;
	}
	return -1;
}

void HandleManager::remove_handle(FrontHandle frontHandle)
{
	this->frontBackendHandleMap.erase(frontHandle);
	if (frontHandle.cursorHandle)
		this->close_globalHandleId(frontHandle.cursorHandle);
	if (frontHandle.preparedHandle)
		this->close_globalHandleId(frontHandle.preparedHandle);
}

int HandleManager::get_backendHandleBaseCursor(unsigned int cursorHandle, BackendHandle& backendHandle)
{
	FrontHandle front;
	front.cursorHandle = cursorHandle;
	front.preparedHandle = 0;

	return this->get_backendHandle(front, backendHandle);
}

int HandleManager::get_backendHandleBasePrepared(unsigned int preparedHandle, BackendHandle& backendHandle)
{
	FrontHandle front;
	front.cursorHandle = 0;
	front.preparedHandle = preparedHandle;

	return this->get_backendHandle(front, backendHandle);
}

void HandleManager::remove_handleBaseCursor(unsigned int cursorHandle)
{
	FrontHandle front;
	front.cursorHandle = cursorHandle;
	front.preparedHandle = 0;
	this->remove_handle(front);
}

void HandleManager::remove_handleBasePrepared(unsigned int preparedHandle)
{
	FrontHandle front;
	front.cursorHandle = 0;
	front.preparedHandle = preparedHandle;
	this->remove_handle(front);
}

void HandleManager::set_backendServer(void* ns)
{
	FBHandleMap::iterator it = this->frontBackendHandleMap.begin();
	for(; it != this->frontBackendHandleMap.end(); ++it) {
		if (it->second.pointer == NULL) {
			it->second.pointer = ns;
		}
	}
}

int HandleManager::get_globalHandleId(unsigned int& handleId)
{
	unsigned int oldLastAllocHandle = this->lastAllocHandle;
	logs(Logger::DEBUG, "oldLastAllocHandle: %d", oldLastAllocHandle);
	for(;;) {
		this->lastAllocHandle = this->lastAllocHandle + 1;
		if (this->lastAllocHandle <= 0)//保证handle都是大于等于1的数
			this->lastAllocHandle = 1;
		if (this->lastAllocHandle == oldLastAllocHandle)
			break;
		logs(Logger::DEBUG, "lastAllocHandle: %d, globalHandleUseFlag: %d",
				this->lastAllocHandle, this->globalHandleUseFlag.size());
		HandleUseFlagMap::iterator it = this->globalHandleUseFlag.find(this->lastAllocHandle);
		if (it == this->globalHandleUseFlag.end()) {
			this->globalHandleUseFlag[this->lastAllocHandle] = (char)1;
			logs(Logger::DEBUG, "lastAllocHandle: %d", this->lastAllocHandle);
			handleId = this->lastAllocHandle;
			return 0;
		}
		logs(Logger::DEBUG, "lastAllocHandle: %d", this->lastAllocHandle);
	}
	logs(Logger::ERR, "no avail handle alloc, error");
	return -1;
}

void HandleManager::close_globalHandleId(unsigned int handle)
{
	this->globalHandleUseFlag.erase(handle);
}

void HandleManager::show_fbHandleMap()
{
	FBHandleMap::iterator it = this->frontBackendHandleMap.begin();
	for (; it != this->frontBackendHandleMap.end(); ++it) {
		logs(Logger::DEBUG, "frontPreparedHandle: %u, frontCursorHandle: %u, "
				"backendPreparedHandle: %u, backendCursorHandle: %u, hashcode: %u, dataVecSize: %d",
				it->first.preparedHandle, it->first.cursorHandle,
				it->second.handle.preparedHandle, it->second.handle.cursorHandle,
				it->second.hashCode, it->second.dataPacketVec.size());
	}
}
