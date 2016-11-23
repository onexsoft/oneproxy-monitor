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
	declare_class_member(std::string, labelName)//数据库标签名称,如果没有配置，则使用ini的节名代替
	declare_class_member(std::string, addr)
	declare_class_member(unsigned int, port)
	declare_class_member(unsigned int, weightValue) //权重值越高，并使用的可能性越高
	declare_class_member(unsigned int, connectNum) //内部使用，不用外部配置。记录当前数据库有多少连接
	declare_class_member(std::string, userName)
	declare_class_member(std::string, password)

	declare_cvt_func(cvtString)
	declare_cvt_func(cvtInt)
public:
	DataBase() {
		this->m_port = 0;
		this->m_weightValue = 0;
		this->m_connectNum = 0;
	}
	DataBase(std::string labelName, std::string addr, unsigned int port, unsigned int weightValue,
			std::string userName, std::string password):
		m_labelName(labelName),
		m_addr(addr),
		m_port(port),
		m_weightValue(weightValue),
		m_connectNum(0),
		m_userName(userName),
		m_password(password)
	{
	}

	bool is_valid() {
		if (this->m_addr.length() > 0 && this->m_port > 0
				&& this->m_userName.length() > 0 && this->m_password.length() > 0)
			return true;
		return false;
	}
	static bool sort_database(DataBase a, DataBase b) {
		return a.get_weightValue() > b.get_weightValue();
	}
};

class DataBaseGroup: public ConfigBase {
	declare_class_member(std::string, labelName)//使用ini的节名
	declare_class_member(std::string, dbMasterGroup)
	declare_class_member(std::string, dbSlaveGroup)
	declare_class_member(std::string, className)
	declare_class_member(unsigned int, frontPort)
	declare_class_member(bool, passwordSeparate)
	declare_class_member(bool, readSlave)
	declare_class_member(bool, useConnectionPool)
	declare_class_member_co(std::vector<DataBase*>, dbMasterGroupVec)
	declare_class_member_co(std::vector<DataBase*>, dbSlaveGroupVec)

	declare_cvt_func(cvtString)
	declare_cvt_func(cvtInt)
	declare_cvt_func(cvtBool)
public:
	DataBaseGroup() {
		this->m_frontPort = 0;
		this->m_passwordSeparate = true;
		this->m_readSlave = true;
		this->m_useConnectionPool = true;
	}

	DataBaseGroup(std::string labelName, std::string dbMasterGroup,
			std::string dbSlaveGroup, std::string className, unsigned int frontPort):
			m_labelName(labelName),
			m_dbMasterGroup(dbMasterGroup),
			m_dbSlaveGroup(dbSlaveGroup),
			m_className(className),
			m_frontPort(frontPort) {
		this->m_passwordSeparate = true;
		this->m_readSlave = true;
		this->m_useConnectionPool = true;
	}

	bool is_valid() {
		if (this->m_dbMasterGroup.length() > 0 && this->m_className.length() > 0) {
			return true;
		}
		return false;
	}

	//根据dbGroupName中的标签从dbVec中查找数据库信息，把地址保存到dbGVec中
	static int set_dataBaseGroupVec(std::vector<DataBase>& dbVec,
			const std::string dbGroupName,
			std::vector<DataBase*>& dbGVec);
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
	declare_class_member(std::string, clientUserName)
	declare_class_member(std::string, clientPassword)
	declare_class_member(int, poolConnCheckActiveTime)
	declare_class_member(int, poolConnTimeoutReleaseTime)
	declare_class_member(int, connectTimeOut)//when connect timeout, oneproxy will close it.

	//global time, one second update once, don't set value by user.
	declare_class_member(u_uint64, globalSecondTime);

	declare_cvt_func(cvtString)
	declare_cvt_func(cvtInt)
	declare_cvt_func(cvtLogger)
	declare_cvt_func(cvtBool)
private:
	Config();
	void add_config(std::vector<ConfigKeyValue>&, std::string, std::string, CVTFunc, SetFunc);
	void default_oneproxyConfig();
	void default_database();
	void default_databaseGroup();
	void default_config();
	void print_config();
	void handle_ports();
	int handle_config();
	int handle_dataBaseGroup();

public:
	static Config* get_config();
	int handle_args(int argc, char* argv[]);
	int loadConfig(std::string filePath);

	void set_database(DataBase database);
	DataBase* get_database(unsigned int index);
	DataBase* get_database();
	unsigned int get_databaseSize();

	void set_dataBaseGroup(DataBaseGroup dataBaseGroup);
	DataBaseGroup* get_dataBaseGroup(unsigned int index);
	unsigned int get_dataBaseGroupSize();

private:
	std::vector<DataBase> dbVector;
	std::vector<DataBaseGroup> dbGroupVector;
	static MutexLock mutexLock;
	std::vector<ConfigKeyValue> oneproxyCfg;
	std::list<std::vector<ConfigKeyValue> > dbInfoCfg;//支持配置多个db信息
	std::list<std::vector<ConfigKeyValue> > dbGroupCfg;//多个组信息
};

#endif /* CONFIG_H_ */
