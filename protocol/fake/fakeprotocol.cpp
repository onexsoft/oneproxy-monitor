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
* @FileName: fakeprotocol.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月23日
*
*/

#include "fakeprotocol.h"

IMPLEMENT_CLASS(FakeProtocol)
FakeProtocol::FakeProtocol() {
	// TODO Auto-generated constructor stub

}

FakeProtocol::~FakeProtocol() {
	// TODO Auto-generated destructor stub
}

void FakeProtocol::protocol_front(Connection& conn)
{
	return;
}

void FakeProtocol::protocol_backend(Connection& conn)
{
	return;
}

bool FakeProtocol::is_currentDatabase(Connection& conn)
{
	return true;
}

void* FakeProtocol::createInstance()
{
	return (void*)new FakeProtocol();
}

void FakeProtocol::destoryInstance()
{
	delete this;
}

int FakeProtocol::get_packetType(StringBuf& packet)
{
	return 0;
}
