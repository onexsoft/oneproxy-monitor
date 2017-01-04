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
 * @FileName: handlemanager.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年10月27日 上午9:49:29
 *  
 */

#ifndef PROTOCOL_HANDLEMANAGER_H_
#define PROTOCOL_HANDLEMANAGER_H_

#include <map>
#include <vector>
#include <iostream>
#include "define.h"
#include "stringbuf.h"

//client prepared or cursor handle data define.
typedef struct _handle_data_t{
	unsigned int preparedHandle;
	unsigned int cursorHandle;
	_handle_data_t() {
		this->preparedHandle = 0;
		this->cursorHandle = 0;
	}
	bool operator< (const _handle_data_t& a) const {
		if (this->preparedHandle < a.preparedHandle && this->cursorHandle <= a.cursorHandle)
			return true;
		else if (this->cursorHandle < a.cursorHandle) {
			return true;
		} else {
			return false;
		}
	}
}HandleData;
typedef HandleData FrontHandle;

typedef struct _data_packet_t{
	void* dataPointer;
	FreeFunc freeFunc;
	_data_packet_t() {
		this->dataPointer = NULL;
		this->freeFunc = NULL;
	}
}DataPacket;

typedef struct _backend_handle_data_t{
	HandleData handle;
	unsigned int hashCode;
	void* pointer; //指向后端连接数据库

	typedef std::vector<DataPacket> DataPacketVector;
	DataPacketVector dataPacketVec; //建立prepared或者游标的数据包

	_backend_handle_data_t() {
		this->hashCode = 0;
		this->pointer = NULL;
	}
	~_backend_handle_data_t() {
		std::vector<DataPacket>::iterator it = dataPacketVec.begin();
		for(; it != dataPacketVec.end(); ++it) {
			if (it->freeFunc != NULL && it->dataPointer != NULL)
				(*(it->freeFunc))(it->dataPointer);
		}
	}
}BackendHandle;

class HandleManager {
public:
	HandleManager();
	~HandleManager();
	int add_handle(unsigned int backendPreparedHandle, unsigned int backendCursorHandle,
			unsigned int hashCode, void* servns, FrontHandle& frontHandle);
	//保存backendHandle,返回frontHandle.
	int add_handle(const BackendHandle& backendHandle, FrontHandle& frontHandle, bool useGlobalId = true);

	int get_backendHandle(FrontHandle frontHandle, BackendHandle& backendHandle);
	FrontHandle* get_frontHandle(unsigned int backendPreparedHandle,
			unsigned int backendCursorHandle, void* servns, unsigned int& hashCode);

	void remove_handle(FrontHandle frontHandle);

	int get_backendHandleBaseCursor(unsigned int cursorHandle, BackendHandle& backendHandle);
	int get_backendHandleBasePrepared(unsigned int preparedHandle, BackendHandle& backendHandle);
	void remove_handleBaseCursor(unsigned int cursorHandle);
	void remove_handleBasePrepared(unsigned int preparedHandle);
	void set_backendServer(void* ns);
private:
	int get_globalHandleId(unsigned int& handleId);
	void close_globalHandleId(unsigned int handle);
	void show_fbHandleMap();

private:
	//client prepared handle mapping onto backendHandleMap.
	typedef std::map<FrontHandle, BackendHandle> FBHandleMap;
	typedef std::map<unsigned int, char> HandleUseFlagMap;
	FBHandleMap frontBackendHandleMap;//前端到后端handle的映射关系

	HandleUseFlagMap globalHandleUseFlag;//记录已经使用过的handle.
	unsigned int lastAllocHandle; //目前可以分配的handle值
};

#endif /* PROTOCOL_HANDLEMANAGER_H_ */
