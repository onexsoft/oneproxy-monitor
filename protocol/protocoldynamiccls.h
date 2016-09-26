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
* @FileName: protocoldynamiccls.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月19日
*
*/

#ifndef PROTOCOL_PROTOCOLDYNAMICCLS_H_
#define PROTOCOL_PROTOCOLDYNAMICCLS_H_

#include "protocolfactory.h"
class ProtocolDynamicCls{
public:
	ProtocolDynamicCls(std::string name, createClass method) {
		ProtocolFactory::sharedClassFactory().registClass(name, method);
	}
};

#define DECLARE_CLASS(className) \
	std::string className##Name; \
	static ProtocolDynamicCls* m_##className##dc ;

#define IMPLEMENT_CLASS(className) \
		ProtocolDynamicCls* className::m_##className##dc = \
		new ProtocolDynamicCls(#className, className::createInstance) ;

#endif /* PROTOCOL_PROTOCOLDYNAMICCLS_H_ */
