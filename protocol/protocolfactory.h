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
* @FileName: protocolfactory.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月19日
*
*/

#ifndef PROTOCOL_PROTOCOLFACTORY_H_
#define PROTOCOL_PROTOCOLFACTORY_H_

#include <string>
#include <map>
#include <iostream>

typedef void* (*createClass)(void);

class ProtocolFactory {
public:
	ProtocolFactory();
	virtual ~ProtocolFactory();

	void* getClassByName(std::string className);
	void registClass(std::string name, createClass method);
	static ProtocolFactory& sharedClassFactory() ;

private:
	std::map<std::string, createClass> m_classMap ;
};

#endif /* PROTOCOL_PROTOCOLFACTORY_H_ */
