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
 * @FileName: opensslutil.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年10月8日 上午11:16:04
 *  
 */

#include "logger.h"
#include "opensslutil.h"
#include "systemapi.h"

OpenSSLUtil::OpenSSLUtil() {
	// TODO Auto-generated constructor stub
	this->m_ssl = NULL;
	this->m_sslCtx = NULL;
	this->m_isClient = true;
}

OpenSSLUtil::OpenSSLUtil(std::string ceriFilePath, std::string keyFilePath)
{
	this->m_ssl = NULL;
	this->m_sslCtx = NULL;
	this->set_ceriFilePath(ceriFilePath);
	this->set_keyFilePath(keyFilePath);
	this->m_isClient = false;
}

OpenSSLUtil::~OpenSSLUtil() {
	// TODO Auto-generated destructor stub
	if (this->m_ssl)
		SSL_free (this->m_ssl);

	if (this->m_sslCtx)
		SSL_CTX_free (this->m_sslCtx);
}

int OpenSSLUtil::init_sslEnv()
{
	SSL_METHOD* method = NULL;

	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_crypto_strings();
	SSLeay_add_all_algorithms();

	if (this->m_isClient) {
		method = (SSL_METHOD*)SSLv23_method();
		this->m_sslCtx = SSL_CTX_new(method);
		if (this->m_sslCtx == NULL) {
			logs(Logger::ERR, "new client ssl ctx error(%s)", ssl_error().c_str());
			return -1;
		}
	} else {//is server
		method = (SSL_METHOD*)SSLv23_method();
		this->m_sslCtx = SSL_CTX_new(method);
		if (this->m_sslCtx == NULL) {
			logs(Logger::ERR, "new server ssl ctx error(%s)", ssl_error().c_str());
			return -1;
		}

		if (this->m_ceriFilePath.length() <= 0 || this->m_keyFilePath.length() <= 0) {
			logs(Logger::ERR, "ceri file path is empty or key file path is empty");
			return -1;
		}
		logs(Logger::ERR, "cerifilepath: %s, keyfilepath: %s",
				this->m_ceriFilePath.c_str(), this->m_keyFilePath.c_str());

		//加载公钥
		if (SSL_CTX_use_certificate_file(this->m_sslCtx, this->m_ceriFilePath.c_str(), SSL_FILETYPE_PEM) <= 0) {
			logs(Logger::ERR, "ssl use certificate file error(%s)", ssl_error().c_str());
			return -1;
		}

		//加载私钥
		if (SSL_CTX_use_PrivateKey_file(this->m_sslCtx, this->m_keyFilePath.c_str(), SSL_FILETYPE_PEM) <= 0) {
			logs(Logger::ERR, "ssl use privatekey error(%s)", ssl_error().c_str());
			return -1;
		}

		//检查私钥和公钥是否匹配
		if (!SSL_CTX_check_private_key(this->m_sslCtx)) {
			logs(Logger::ERR, "check private key error(%s)", ssl_error().c_str());
			return -1;
		}
	}

	this->m_ssl = SSL_new(this->m_sslCtx);
	if (this->m_ssl == NULL) {
		logs(Logger::ERR, "new ssl error(%s)", ssl_error().c_str());
		return -1;
	}

	return 0;
}

int OpenSSLUtil::accept_sslFd(unsigned int fd, handle_data_callback cb)
{
	assert(this->m_ssl != NULL);

	SSL_set_fd(this->m_ssl, fd);
	BIO_set_callback(this->m_ssl->rbio, cb);
	SSL_accept(this->m_ssl);
	return 0;
}

int OpenSSLUtil::connect_sslFd(unsigned int fd, handle_data_callback cb)
{
	assert(this->m_ssl != NULL);
	uif(!SSL_set_fd(this->m_ssl, fd)) {
		logs(Logger::ERR, "set fd error(%s)", ssl_error().c_str());
		return -1;
	}

	BIO_set_callback(this->m_ssl->rbio, cb);//rbio and wbio is the same io pointer

	int ret = 0;
	if ((ret = SSL_connect(this->m_ssl)) <= 0) {//when 1 is success.
		logs(Logger::ERR, "ssl connect error(%s)", ssl_error().c_str());
		return -1;
	}
	return 0;
}

void OpenSSLUtil::set_callback(handle_data_callback cb)
{
	assert(this->m_ssl != NULL);
	BIO_set_callback(this->m_ssl->rbio, cb);
}

int OpenSSLUtil::close_sslFd()
{
	assert(this->m_ssl != NULL);
	SSL_shutdown(this->m_ssl);
	return 0;
}

int OpenSSLUtil::read_data(StringBuf& data)
{
	assert(this->m_ssl != NULL);

	data.reallocMem(4096);
	int ret = 0;
	if ((ret = SSL_read(this->m_ssl, (char*)(data.addr() + data.get_length()), data.get_remailAllocLen())) <= 0) {
		ret = this->get_sslErrorCode();
		if (ret == SSL_ERROR_WANT_READ) {
			return 1;
		}
		logs(Logger::ERR, "ssl read error(%d)", ret);
		return -1;
	}
	data.set_length(data.get_length() + ret);
	return 0;
}

int OpenSSLUtil::write_data(StringBuf& data)
{
	assert(this->m_ssl != NULL);

	uif (SSL_write(this->m_ssl, (char*)(data.addr() + data.get_offset()), data.get_remailLength()) < 0) {
		if (this->get_sslErrorCode() == SSL_ERROR_WANT_WRITE) {
			return 1;
		}
		logs(Logger::ERR, "ssl write data error(%s)", ssl_error().c_str());
		return -1;
	}
	return 0;
}

std::string OpenSSLUtil::ssl_error()
{
	std::string error;
	unsigned long en = ERR_get_error();
	char szErrMsg[1024] = {0};

	error.append(ERR_error_string(en, szErrMsg));

	return error;
}

int OpenSSLUtil::get_sslErrorCode()
{
	assert(this->m_ssl != NULL);
	int icode = -1;
	int iret = SSL_get_error(this->m_ssl, icode);
	return iret;
}
