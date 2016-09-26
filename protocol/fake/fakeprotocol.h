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
* @FileName: fakeprotocol.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月23日
*
*/

#ifndef PROTOCOL_FAKE_FAKEPROTOCOL_H_
#define PROTOCOL_FAKE_FAKEPROTOCOL_H_

#include "protocolbase.h"

class FakeProtocol : public ProtocolBase {
public:
	FakeProtocol();
	virtual ~FakeProtocol();
	virtual void protocol_front(Connection& conn);
	virtual void protocol_backend(Connection& conn);
	virtual bool is_currentDatabase(Connection& conn);
	static void* createInstance();
	virtual void destoryInstance();
	virtual int get_packetType(StringBuf& packet);
private:
	DECLARE_CLASS(FakeProtocol)
};

#endif /* PROTOCOL_FAKE_FAKEPROTOCOL_H_ */
