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
* @FileName: keepalive.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月8日
*
*/

#ifndef UTIL_KEEPALIVE_H_
#define UTIL_KEEPALIVE_H_

class KeepAlive {
public:
	KeepAlive();
	virtual ~KeepAlive();


	int keepalive(int *child_exit_status, const char *pid_file);

	/**
	 * @desc 信号处理函数
	 * @param sig 信号值
	 * @return 成功返回0， 否则返回-1.
	 * **/
	static void handle_signal(int sig);
};

#endif /* UTIL_KEEPALIVE_H_ */
