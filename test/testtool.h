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
 * @FileName: testtool.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年10月18日 上午9:47:11
 *  
 */

#ifndef TEST_TESTTOOL_H_
#define TEST_TESTTOOL_H_

#include "cpptest.h"

class TestTool : public Test::Suite{
public:
	TestTool() {
		TEST_ADD(TestTool::test_wstringFormat);
//		TEST_ADD(TestTool::test_string2wstring);
//		TEST_ADD(TestTool::test_wstring2string);
	}
	virtual ~TestTool() {

	}
private:
	void test_wstringFormat();
	void test_string2wstring();
	void test_wstring2string();
};

#endif /* TEST_TESTTOOL_H_ */
