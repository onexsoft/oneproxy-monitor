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
* @FileName: config.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月10日
*
*/

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <strings.h>

#include "logger.h"
#include "mutexlock.h"
#include "define.h"

class ConfigBase{};
typedef struct config_key_value_t ConfigKeyValue;
typedef void (ConfigBase::*CVTFunc)(std::string value, ConfigKeyValue& cf);
typedef void (ConfigBase::*SetFunc)();
#define addConfig(key, defaultValue, cvtFunc, setFunc) {key, defaultValue, (CVTFunc)cvtFunc, (SetFunc)setFunc}
struct config_key_value_t{
	std::string key;
	std::string defaultValue;
	CVTFunc cvtFunc;
	SetFunc setFunc; //only save function address
	config_key_value_t(std::string key, std::string defaultValue, CVTFunc cvtFunc, SetFunc setFunc) {
		this->key = key;
		this->defaultValue = defaultValue;
		this->cvtFunc = cvtFunc;
		this->setFunc = setFunc;
	}
};

#define declare_cvt_func(funcName) public: void funcName (std::string value, ConfigKeyValue& cf);
class DataBase: public ConfigBase{
	declare_class_member(std::string, addr)
	declare_class_member(unsigned int, port)
	declare_class_member(unsigned int, frontPort)//当为0时，前端数据都可以发送到这个数据库
	declare_class_member(std::string, className)
	declare_class_member(std::string, companyName)

	declare_cvt_func(cvtString)
	declare_cvt_func(cvtInt)
public:
	DataBase(){}
	DataBase(std::string companyName, std::string className, std::string addr, unsigned int port):
		m_addr(addr),
		m_port(port),
		m_frontPort(0),
		m_className(className),
		m_companyName(companyName)
	{

	}

	bool is_valid() {
		if (this->m_addr.length() > 0 && this->m_port > 0 && this->m_className.length() > 0)
			return true;
		return false;
	}
};

#define config() Config::get_config()
#define decl
class Config: public ConfigBase{
	declare_class_member(unsigned int, maxConnectNum)//oneproxy 同时最大处理量
	declare_class_member(Logger::loggerLevel, loggerLevel)
	declare_class_member(int, tryConnServerTimes)//尝试连接服务器的次数，如果达到这个次数还没有成功，则关闭客户端的连接
	declare_class_member(std::string, oneproxyAddr)
	declare_class_member(std::string, oneproxyPort)
	declare_class_member_co(std::set<unsigned int>, oneproxyPortSet)
	declare_class_member(bool, dumpData)
	declare_class_member(bool, logSql)
	declare_class_member(bool, keepAlive)
	declare_class_member(std::string, httpServerAddr)
	declare_class_member(std::string, httpServerTitle)
	declare_class_member(int, httpServerPort)
	declare_class_member(std::string, logFilePath)
	declare_class_member(std::string, pidFilePath)
	declare_class_member(std::string, vipIfName)
	declare_class_member(std::string, vipAddress)
	declare_class_member(int, threadNum)

	declare_cvt_func(cvtString)
	declare_cvt_func(cvtInt)
	declare_cvt_func(cvtLogger)
	declare_cvt_func(cvtBool)
private:
	Config();
	void add_config(std::vector<ConfigKeyValue>&, std::string, std::string, CVTFunc, SetFunc);
	void default_oneproxyConfig();
	void default_database();
	void default_config();
	void print_config();
	void handle_ports();
	int verify_config();

public:
	static Config* get_config();
	int handle_args(int argc, char* argv[]);
	int loadConfig(std::string filePath);
	void set_database(DataBase database);
	DataBase* get_database(unsigned int index);
	DataBase* get_database();
	unsigned int get_databaseSize();

private:
	std::vector<DataBase> dbvector;
	static MutexLock mutexLock;
	std::vector<ConfigKeyValue> oneproxycfg;
	std::list<std::vector<ConfigKeyValue> > dbinfocfg;//支持配置多个db信息
};

#endif /* CONFIG_H_ */
