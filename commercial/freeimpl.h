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
 * @FileName: FreeImpl.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年12月7日 下午3:05:08
 *  
 */

#ifndef FreeImpl_FreeImpl_H_
#define FreeImpl_FreeImpl_H_

#include "connection.h"
#include "define.h"

class FreeImpl {
public:
	FreeImpl();
	virtual ~FreeImpl();

	virtual int get_databaseFromGroup(Connection& con);
};

#endif /* FreeImpl_FreeImpl_H_ */
