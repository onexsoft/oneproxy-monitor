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
 * @FileName: testtcpclient.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年9月27日 上午11:36:43
 *  
 */

#ifndef TEST_TESTTCPCLIENT_H_
#define TEST_TESTTCPCLIENT_H_

#include "cpptest.h"

class TestTcpClient: public Test::Suite {
public:
	TestTcpClient() {
		TEST_ADD(TestTcpClient::test_getBackendConnection);
		TEST_ADD(TestTcpClient::test_sqlserver);
	}
private:
	void test_getBackendConnection();
	void test_sqlserver();
};

#endif /* TEST_TESTTCPCLIENT_H_ */
