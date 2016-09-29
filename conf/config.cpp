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
* @FileName: config.cpp
* @Description:
*  	1. 增加构造函数config()中的配置信息
*  	2. 增加print_config中对新加配置的打印信息
*  	3. 修改handle_args函数， 添加新配置的支持。
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月10日
*
*/

#include "config.h"
#include "simpleini.h"
#include "tool.h"
#include "systemapi.h"

#include <getopt.h>

#define IMPLEMENT_CVTBOOL_FUNC(className, funcName) \
void className::funcName(std::string value, ConfigKeyValue& cf) \
{ \
	typedef void (className::*SetBoolValue)(bool); \
	SetBoolValue func = (SetBoolValue)cf.setFunc; \
	if ((strncasecmp_s(value, std::string("true")) == 0) \
			|| (strncasecmp_s(value, std::string("1")) == 0)) {\
		(this->*func)(true);\
	} else { \
		(this->*func)(false); \
	}\
}

#define IMPLEMENT_CVTINT_FUNC(className, funcName) \
void className::funcName(std::string value, ConfigKeyValue& cf) \
{ \
	typedef void (className::*SetIntValue)(int); \
	SetIntValue func = (SetIntValue)cf.setFunc; \
	int v = atoi(value.c_str()); \
	(this->*func)(v); \
}

#define IMPLEMENT_CVTLOGGER_FUNC(className, funcName) \
void className::funcName(std::string value, ConfigKeyValue& cf) \
{ \
	typedef void(className::*SetLoggerValue)(Logger::loggerLevel); \
	SetLoggerValue func = (SetLoggerValue)cf.setFunc; \
	if (strncasecmp_s(value, std::string("debug")) == 0) { \
		(this->*func)(Logger::DEBUG); \
	} else if (strncasecmp_s(value, std::string("info")) == 0) { \
		(this->*func)(Logger::INFO); \
	} else if (strncasecmp_s(value, std::string("warning")) == 0) {\
		(this->*func)(Logger::WARNING); \
	} else if (strncasecmp_s(value, std::string("error")) == 0) { \
		(this->*func)(Logger::ERR); \
	} else if (strncasecmp_s(value, std::string("fatal")) == 0) { \
		(this->*func)(Logger::FATAL); \
	} else { \
		logs(Logger::ERR, "unkown level(%s)", value.c_str()); \
	} \
}

#define IMPLEMENT_CVTSTRING_FUNC(className, funcName) \
void className::funcName(std::string value, ConfigKeyValue& cf) \
{ \
	typedef void (className::*SetStringValue)(std::string); \
	SetStringValue func = (SetStringValue)cf.setFunc; \
	(this->*func)(value); \
}

IMPLEMENT_CVTSTRING_FUNC(DataBase, cvtString)
IMPLEMENT_CVTINT_FUNC(DataBase, cvtInt)

Config::Config()
{
	//add oneproxy default config.
#define add_oneproxyConfig(key, defaultv, cvtf, setf) add_config(this->oneproxycfg, key, defaultv, (CVTFunc)cvtf, (SetFunc)setf)
	add_oneproxyConfig("logfile", "oneproxy_log.log", &Config::cvtString, &Config::set_logFilePath);
	add_oneproxyConfig("pidfile", "oneproxy_pid.pid", &Config::cvtString, &Config::set_pidFilePath);
	add_oneproxyConfig("listen_addr", "0.0.0.0", &Config::cvtString, &Config::set_oneproxyAddr);
	add_oneproxyConfig("listen_port", "9999", &Config::cvtString, &Config::set_oneproxyPort);
	add_oneproxyConfig("httpserver_addr", "0.0.0.0", &Config::cvtString, &Config::set_httpServerAddr);
	add_oneproxyConfig("httpserver_port", "8080", &Config::cvtInt, &Config::set_httpServerPort);
	add_oneproxyConfig("log_level", "error", &Config::cvtLogger, &Config::set_loggerLevel);
	add_oneproxyConfig("data_dump", "false", &Config::cvtBool, &Config::set_dumpData);
	add_oneproxyConfig("log_sql", "false", &Config::cvtBool, &Config::set_logSql);
	add_oneproxyConfig("tryConnServerTimes", "3", &Config::cvtInt, &Config::set_tryConnServerTimes);
	add_oneproxyConfig("maxConnectNum", "2000", &Config::cvtInt, &Config::set_maxConnectNum);
	add_oneproxyConfig("keepalive", "false", &Config::cvtBool, &Config::set_keepAlive);
	add_oneproxyConfig("vip_ifname", "", &Config::cvtString, &Config::set_vipIfName);
	add_oneproxyConfig("vip_address", "", &Config::cvtString, &Config::set_vipAddress);
	add_oneproxyConfig("threadnum", "0", &Config::cvtInt, &Config::set_threadNum);
#undef add_oneproxyConfig

#define add_dbConfig(db, key, defaultv, cvtf, setf) add_config(db, key, defaultv, (CVTFunc)cvtf, (SetFunc)setf)
	//增加一个fake数据处理对象,此不能删除
	std::vector<ConfigKeyValue> fakedb;
	add_dbConfig(fakedb, "host", "127.0.0.1", &DataBase::cvtString, &DataBase::set_addr);
	add_dbConfig(fakedb, "port", "1433", &DataBase::cvtInt, &DataBase::set_port);
	add_dbConfig(fakedb, "frontPort", "0", &DataBase::cvtInt, &DataBase::set_frontPort);
	add_dbConfig(fakedb, "className", "FakeProtocol", &DataBase::cvtString, &DataBase::set_className);
	add_dbConfig(fakedb, "companyName", "fake database", &DataBase::cvtString, &DataBase::set_companyName);
	add_dbConfig(fakedb, "priority", "-999", &DataBase::cvtInt, &DataBase::set_priority);
	this->dbinfocfg.push_back(fakedb);

	//如果需要增加其他的数据库，则只需要重复fake database的部分即可。
	//add sql server default config.
	std::vector<ConfigKeyValue> sqlserver;
	add_dbConfig(sqlserver, "host", "127.0.0.1", &DataBase::cvtString, &DataBase::set_addr);
	add_dbConfig(sqlserver, "port", "1433", &DataBase::cvtInt, &DataBase::set_port);
	add_dbConfig(sqlserver, "frontPort", "0", &DataBase::cvtInt, &DataBase::set_frontPort);
	add_dbConfig(sqlserver, "className", "SSProtocol", &DataBase::cvtString, &DataBase::set_className);
	add_dbConfig(sqlserver, "companyName", "sql server", &DataBase::cvtString, &DataBase::set_companyName);
	add_dbConfig(sqlserver, "priority", "0", &DataBase::cvtInt, &DataBase::set_priority);
	this->dbinfocfg.push_back(sqlserver);

	//add postgresql database default config.
	std::vector<ConfigKeyValue> pgserver;
	add_dbConfig(pgserver, "host", "127.0.0.1", &DataBase::cvtString, &DataBase::set_addr);
	add_dbConfig(pgserver, "port", "5432", &DataBase::cvtInt, &DataBase::set_port);
	add_dbConfig(pgserver, "frontPort", "0", &DataBase::cvtInt, &DataBase::set_frontPort);
	add_dbConfig(pgserver, "className", "PGProtocol", &DataBase::cvtString, &DataBase::set_className);
	add_dbConfig(pgserver, "companyName", "postgresql", &DataBase::cvtString, &DataBase::set_companyName);
	add_dbConfig(pgserver, "priority", "0", &DataBase::cvtInt, &DataBase::set_priority);
	this->dbinfocfg.push_back(pgserver);
#undef add_dbConfig
}

void Config::add_config(std::vector<ConfigKeyValue>& db, std::string key, std::string dvalue, CVTFunc cf, SetFunc sf)
{
	db.push_back(ConfigKeyValue(key, dvalue, cf, sf));
}

void Config::default_oneproxyConfig()
{
	std::vector<ConfigKeyValue>::iterator it = this->oneproxycfg.begin();
	for(; it != this->oneproxycfg.end(); ++it) {
		CVTFunc func = (*it).cvtFunc;
		(this->*func)((*it).defaultValue, (*it));
	}
}

void Config::default_database()
{
	std::list<std::vector<ConfigKeyValue> >::iterator it = this->dbinfocfg.begin();
	for (; it != this->dbinfocfg.end(); ++it) {
		DataBase db;

		std::vector<ConfigKeyValue>& dbcfg = (*it);
		std::vector<ConfigKeyValue>::iterator dbit = dbcfg.begin();
		for (; dbit != dbcfg.end(); ++dbit) {
			CVTFunc func = (*dbit).cvtFunc;
			(db.*func)((*dbit).defaultValue, (*dbit));
		}
		this->set_database(db);
	}
}

void Config::default_config()
{
	config()->default_oneproxyConfig();
	config()->default_database();
}

void Config::print_config()
{
	logs(Logger::INFO, "maxConnectNum: %d", (int)this->m_maxConnectNum);
	logs(Logger::INFO, "loggerLevel: %d", this->m_loggerLevel);
	logs(Logger::INFO, "tryConnServerTimes: %d", this->m_tryConnServerTimes);
	logs(Logger::INFO, "oneproxyAddr: %s", this->m_oneproxyAddr.c_str());
	logs(Logger::INFO, "oneproxyPort: %s", this->m_oneproxyPort.c_str());
	{
		std::set<unsigned int>::iterator it = this->m_oneproxyPortSet.begin();
		for (; it != this->m_oneproxyPortSet.end(); ++it) {
			logs(Logger::INFO, "port: %u", *it);
		}
	}
	logs(Logger::INFO, "dumpData: %d", this->m_dumpData);
	logs(Logger::INFO, "logSql: %d", this->m_logSql);
	logs(Logger::INFO, "httpServerAddr: %s", this->m_httpServerAddr.c_str());
	logs(Logger::INFO, "httpServerPort: %d", this->m_httpServerPort);
	logs(Logger::INFO, "logFilePath: %s", this->m_logFilePath.c_str());
	logs(Logger::INFO, "pidFilePath: %s", this->m_pidFilePath.c_str());
	logs(Logger::INFO, "keepAlive: %d", this->m_keepAlive);
	logs(Logger::INFO, "vipIfName: %s", this->m_vipIfName.c_str());
	logs(Logger::INFO, "vipAddress: %s", this->m_vipAddress.c_str());
	logs(Logger::INFO, "threadNum: %d", this->m_threadNum);

	std::vector<DataBase>::iterator it = dbvector.begin();
	for (; it != dbvector.end(); ++it) {
		logs(Logger::INFO, "addr: %s, port: %d, frontPort: %d, className: %s, companyName: %s, priority: %d",
				it->get_addr().c_str(), it->get_port(), it->get_frontPort(),
				it->get_className().c_str(), it->get_companyName().c_str(), it->get_priority());
	}
}

void Config::handle_ports()
{

	if (this->m_oneproxyPort.size() <= 0)
		return;

	size_t pos = 0;
	do {
		size_t curPos = this->m_oneproxyPort.find_first_of(',', pos);
		if (curPos == this->m_oneproxyPort.npos) {
			curPos = this->m_oneproxyPort.length();
		}

		unsigned int port = (unsigned int)atoi(this->m_oneproxyPort.substr(pos, curPos - pos).c_str());
		if (port > 0) {
			this->m_oneproxyPortSet.insert(port);
		}

		if (curPos >= this->m_oneproxyPort.length())
			break;

		pos = curPos + 1;
	} while(1);
}

int Config::handle_config()
{
	unsigned int limitFiles = 0;
	if (SystemApi::system_limitFileNum(limitFiles)) {
		logs(Logger::ERR, "get limitFile error");
		return -1;
	}
	unsigned int tlf = limitFiles /2;

	//预留20个文件描述给http server和监听套接字等使用
	if (tlf < this->get_maxConnectNum() && tlf > 20) {
		this->set_maxConnectNum(tlf - 20);
	} else if (tlf < 20){
		this->set_maxConnectNum(0);
	}

	unsigned int cpuNum = SystemApi::system_cpus();
	if (this->get_threadNum() <= 0 && cpuNum > 0) {
		this->set_threadNum(cpuNum);
	}

	//排序到dbvector的协议调用顺序
	std::sort(this->dbvector.begin(), this->dbvector.end(), DataBase::sort_database);

	return 0;
}

MutexLock Config::mutexLock;
Config* Config::get_config()
{
	static Config* config = NULL;
	if (config == NULL) {
		Config::mutexLock.set_name("config_lock");
		Config::mutexLock.lock();
		if (config == NULL) {
			config = new Config();
		}
		Config::mutexLock.unlock();
	}
	return config;
}

int Config::handle_args(int argc, char* argv[])
{
	int long_idx = 0;
	int c;

	static const char usage_str[] =
		"Usage: %s [OPTION]\n"
		//"  -d, --daemon           Run in background (as a daemon)\n"
		"  -q, --quiet            Run quietly\n"
		"  -v, --verbose          Increase verbosity\n"
		"  -V, --version          Show version\n"
		"  -h, --help             Show this help screen and exit\n"
		"  -f, --file             Config file path\n"
		"--oneproxy_address       Oneproxy listen address(default:127.0.0.1)\n"
		"--oneproxy_port          Oneproxy listen port(default:9999); when have many ports, use comma separate\n"
		"--httpserver_address     Http server listen address(default:127.0.0.1)\n"
		"--httpserver_port        Http server listen port(default:8080)\n"
		"--database_host          Database listen address(default:127.0.0.1)\n"
		"--database_port          Database listen port(default:sqlserver 1433)\n"
		"--database_classname     ClassName of handle database protocol(default:FakeProtocol)\n"
		"--database_companyname   The company name of database (default: fake database)\n "
		"--maxconnectnum          The number of oneproxy connection to database(default:2000)\n"
		"--keepalive              keep the process alive\n"
		"--vip_ifname             the vip network adapter name, for example: eth0:0\n"
		"--vip_address            the vip address\n"
		"--threadnum              the number of worker threads\n"
			;

	static const struct option long_options[] = {
			{"oneproxy_address", required_argument, NULL, 'a'},
			{"httpserver_address", required_argument, NULL, 'A'},
			{"database_host", required_argument, NULL, 'b'},
			{"database_port", required_argument, NULL, 'c'},
			{"daemon", no_argument, NULL, 'd'},
			{"dump_data", no_argument, NULL, 'D'},
			{"database_classname", required_argument, NULL, 'e'},
			{"file", required_argument, NULL, 'f'},
			{"vip_ifname", required_argument, NULL, 'g'},
			{"vip_address", required_argument, NULL, 'G'},
			{"help", no_argument, NULL, 'h'},
			{"keepalive", no_argument, NULL, 'k'},
			{"maxconnectnum", required_argument, NULL, 'm'},
			{"oneproxy_port", required_argument, NULL, 'p'},
			{"httpserver_port", required_argument, NULL, 'P'},
			{"quiet", no_argument, NULL, 'q'},
			{"log_sql", no_argument, NULL, 's'},
			{"threadnum", required_argument, NULL, 't'},
			{"verbose", no_argument, NULL, 'v'},
			{"version", no_argument, NULL, 'V'},
			{NULL, 0, NULL, 0},
	};

	//先载入默认配置，再根据其他配置针对默认配置的修改
	config()->default_oneproxyConfig();

	if (argc > 1) {
		DataBase db;
		while ((c = getopt_long(argc, argv, "qvhkdVDsf:", long_options, &long_idx)) != -1) {
			switch (c) {
			case 'v':
				config()->set_loggerLevel(Logger::INFO);
				break;
			case 'V':
				printf("%s\n", oneproxy_version());
				exit(0);
				break;
			case 'd':
				break;
			case 'q':
				config()->set_loggerLevel(Logger::ERR);
				break;
			case 'f':
				config()->loadConfig(optarg);
				break;
			case 'g':
				config()->set_vipIfName(std::string(optarg));
				break;
			case 'G':
				config()->set_vipAddress(std::string(optarg));
				break;
			case 'D':
				config()->set_dumpData(true);
				break;
			case 's':
				config()->set_logSql(true);
				break;
			case 'a':
				config()->set_oneproxyAddr(std::string(optarg));
				break;
			case 'p':
				config()->set_oneproxyPort(std::string(optarg));
				break;
			case 'A':
				config()->set_httpServerAddr(std::string(optarg));
				break;
			case 'P':
				config()->set_httpServerPort(atoi(optarg));
				break;
			case 'b':
				db.set_addr(std::string(optarg));
				break;
			case 'c':
				db.set_port(atoi(optarg));
				break;
			case 'e':
				db.set_className(std::string(optarg));
				break;
			case 'm':
				config()->set_maxConnectNum(atoi(optarg));
				break;
			case 'k':
				config()->set_keepAlive(true);
				break;
			case 't':
				config()->set_threadNum(atoi(optarg));
				break;
			case 'h':
				printf("%s\n", usage_str);
				exit(0);
				break;
			default:
				printf("%s\n", usage_str);
				exit(1);
			}
		}

		if (db.is_valid()) {
			config()->set_database(db);
		} else if (config()->get_databaseSize() <= 0) {
			config()->default_database();//set default database
		}
	} else {
		// 寻找配置文件
		do {
			std::string configFilePath;
#ifdef _WIN32
			configFilePath = Tool::search_oneFile(std::string(".\\"), std::string("*.ini"));
#else
			configFilePath = Tool::search_oneFile(std::string("./"), std::string("*.ini"));
#endif
			if (configFilePath.length() <= 0) {
				logs(Logger::INFO, "use default config");
				config()->default_database();
				break;
			}

			//载入配置文件
			if (config()->loadConfig(configFilePath)) {
				logs(Logger::FATAL, "load config file error, exit.");
				return -1;
			}

			//如果配置文件中没有配置数据库，则使用默认数据库
			if(config()->get_databaseSize() <= 0)
				config()->default_database();
		} while(0);
	}

	//处理多个端口号的情况。
	this->handle_ports();
	if (this->handle_config()) {
		logs(Logger::ERR, "verify config error");
		return -1;
	}

	config()->print_config();

	//set logger level
	logs_setLevel(config()->get_loggerLevel());
	logs_setDumpData(config()->get_dumpData());
	logs_setLogSql(config()->get_logSql());
	logs_setLogFile(config()->get_logFilePath());

	return 0;
}

int Config::loadConfig(std::string filePath)
{
	CSimpleIniA ini;
	ini.SetUnicode();
	if (ini.LoadFile(filePath.c_str()) != SI_OK) {
		logs(Logger::ERR, "load file error");
		return -1;
	}

	CSimpleIniA::TNamesDepend sections;
	ini.GetAllSections(sections);
	CSimpleIniA::TNamesDepend::iterator sit = sections.begin();
	for (; sit != sections.end(); ++sit) {
		if (strncmp_c(sit->pItem, "oneproxy") == 0) {
			//load oneproxy section
			std::vector<ConfigKeyValue>::iterator it = this->oneproxycfg.begin();
			for (; it != this->oneproxycfg.end(); ++it) {
				const char* value = ini.GetValue((const char*)sit->pItem, (*it).key.c_str(), (*it).defaultValue.c_str());
				CVTFunc func = (*it).cvtFunc;
				(this->*func)(std::string(value), (*it));
			}
		} else {
			//load database section
			const char* dbClassNameKey = "className";
			const char* dbClassNameValue = "FakeProtocol";//default Name

			//1. read className
			const char* className = ini.GetValue((const char*)sit->pItem, dbClassNameKey, dbClassNameValue);
			if (className == NULL) {//必须配置className,否则无法识别处理类
				continue;
			}
			if (this->dbinfocfg.size() <= 0)//必须有默认配置，至少一个db的默认配置。
				continue;

			//2. 根据className查找默认配置
			std::vector<ConfigKeyValue>* dbDefaultConfig = NULL;
			std::list<std::vector<ConfigKeyValue> >::iterator firstDBCfg = this->dbinfocfg.begin();
			for(; firstDBCfg != this->dbinfocfg.end(); ++firstDBCfg) {
				std::vector<ConfigKeyValue>::iterator tdbit = (*firstDBCfg).begin();
				for (; tdbit != (*firstDBCfg).end(); ++tdbit) {
					ConfigKeyValue& ckv = (*tdbit);
					if ((strncasecmp_s(ckv.key, std::string(dbClassNameKey)) == 0)
							&& (strncasecmp_s(ckv.defaultValue, std::string(className)) == 0)) {
						dbDefaultConfig = &(*firstDBCfg);
						break;
					}
				}
				if (dbDefaultConfig != NULL)
					break;
			}

			//3. 根据默认配置读取数据库配置信息
			bool has_defaultConfig = true;
			if (dbDefaultConfig == NULL) {
				has_defaultConfig = false;
				dbDefaultConfig = &this->dbinfocfg.front();
			}
			DataBase db;
			std::vector<ConfigKeyValue>::iterator dbit = dbDefaultConfig->begin();
			for (; dbit != dbDefaultConfig->end(); ++dbit) {
				char* value = NULL;
				if (has_defaultConfig) {
					value = (char*)ini.GetValue((const char*)sit->pItem, (*dbit).key.c_str(), (*dbit).defaultValue.c_str());
				} else {
					value = (char*)ini.GetValue((const char*)sit->pItem, (*dbit).key.c_str(), (const char*)"");
				}

				CVTFunc func = (*dbit).cvtFunc;
				(db.*func)(std::string(value), (*dbit));
			}
			uif (db.get_priority() < 0) {//从配置读取的优先级不能小于0.
				db.set_priority(0);
			}
			this->set_database(db);
		}
	}
	return 0;
}

void Config::set_database(DataBase db) {
	this->dbvector.push_back(db);
}

DataBase * Config::get_database() {
	if (this->dbvector.size()) {
		return &this->dbvector.at(0);
	}
	return NULL;
}

unsigned int Config::get_databaseSize() {
	return this->dbvector.size();
}

DataBase* Config::get_database(unsigned int index) {
	if (index >= this->get_databaseSize()) {
		return NULL;
	}
	return &this->dbvector[index];
}

IMPLEMENT_CVTSTRING_FUNC(Config, cvtString)
IMPLEMENT_CVTINT_FUNC(Config, cvtInt)
IMPLEMENT_CVTLOGGER_FUNC(Config, cvtLogger)
IMPLEMENT_CVTBOOL_FUNC(Config, cvtBool)
