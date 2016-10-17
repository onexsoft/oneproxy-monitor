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
 * @FileName: opensslutil.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年10月8日 上午11:16:03
 *  
 */

#ifndef UTIL_OPENSSLUTIL_H_
#define UTIL_OPENSSLUTIL_H_

#include "define.h"
#include "stringbuf.h"

#include <string>
#include <iostream>
#ifdef __WIN32
#include <winsock2.h>
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

typedef long (*handle_data_callback) (struct bio_st *bio, int oper, const char *data, int dataLen, long argl, long ret);
class OpenSSLUtil {
public:
	OpenSSLUtil();
	OpenSSLUtil(std::string ceriFilePath, std::string keyFilePath);
	virtual ~OpenSSLUtil();
	int init_sslEnv();
	int accept_sslFd(unsigned int fd, handle_data_callback cb);
	int connect_sslFd(unsigned int fd, handle_data_callback cb);
	void set_callback(handle_data_callback cb);
	int close_sslFd();
	int read_data(StringBuf& data);
	int write_data(StringBuf& data);
private:
	std::string ssl_error();
	int get_sslErrorCode();
private:
	declare_class_member(SSL*, ssl)
	declare_class_member(bool, isClient)
	declare_class_member(SSL_CTX*, sslCtx)
	declare_class_member_co(std::string, ceriFilePath)
	declare_class_member_co(std::string, keyFilePath)
};

#endif /* UTIL_OPENSSLUTIL_H_ */
