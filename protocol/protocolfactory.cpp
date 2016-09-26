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
* @FileName: protocolfactory.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月19日
*
*/

#include "protocolfactory.h"

ProtocolFactory::ProtocolFactory() {
	// TODO Auto-generated constructor stub

}

ProtocolFactory::~ProtocolFactory() {
	// TODO Auto-generated destructor stub
}

void* ProtocolFactory::getClassByName(std::string className)
{
    std::map<std::string, createClass>::const_iterator iter ;

    iter = m_classMap.find(className) ;
    if ( iter == m_classMap.end() )
        return NULL ;
    else
        return iter->second() ;
}

void ProtocolFactory::registClass(std::string name, createClass method)
{
    m_classMap.insert(std::pair<std::string, createClass>(name, method)) ;
}

ProtocolFactory& ProtocolFactory::sharedClassFactory()
{
    static ProtocolFactory _sharedClassFactory ;
    return _sharedClassFactory ;
}
