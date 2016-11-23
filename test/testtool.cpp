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
 * @FileName: testtool.cpp
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年10月18日 上午9:47:11
 *  
 */

#include "testtool.h"
#include "tool.h"
#include "logger.h"
#include "sslgen.h"

#define CERT_COUNTRY_NAME           "FR"
#define CERT_STATE_OR_PROVINCE_NAME "IDF"
#define CERT_LOCALITY_NAME          "Paris"
#define CERT_ORGANIZATION_NAME      "GITHUB"
#define CERT_ORGANIZATION_UNIT_NAME "IT"

void TestTool::test_wstringFormat()
{
	/*instanciate certificate generation lib*/
	SSLGen ssl_gen;

	/* get system time for date start*/
	time_t systime;
	struct tm *sys_time;
	time(&systime);
	sys_time=localtime(&systime);

	/* set end date to 30/08/2019 00:00:00 (current timezone)*/
	struct tm  date_end;
	date_end.tm_year = 2019 - 1900;
	date_end.tm_mon = 8 - 1;
	date_end.tm_mday = 30;
	date_end.tm_hour = 0;
	date_end.tm_min = 0;
	date_end.tm_sec = 0;
	date_end.tm_isdst = sys_time->tm_isdst;

	/*set certificate entries*/
	cert_entries entries;
//	entries.country_name = CERT_COUNTRY_NAME;
//	entries.state_province_name = CERT_STATE_OR_PROVINCE_NAME;
//	entries.locality_name = CERT_LOCALITY_NAME;
//	entries.organization_name = CERT_ORGANIZATION_NAME;
//	entries.organizational_unit_name = CERT_ORGANIZATION_UNIT_NAME;

	entries.country_name = (char*)"";
	entries.state_province_name = (char*)"";
	entries.locality_name = (char*)"";
	entries.organization_name = (char*)"";
	entries.organizational_unit_name = (char*)"";

	/* generate public/private key (we want PEM + PKCS12 format) + output is retrieved through input pointer + file output name*/

	/*set output cert as pem certificate (default). If you set file output name. Cert will be written under these files*/
	ssl_gen.setOutputPEM(true, (char*)"test.crt", (char*)"test.key");

	/*set output cert as p12 certificate. If you set file output name. Cert will be written under these files*/
	ssl_gen.setOutputP12(true, (char*)"test.p12");

	certificate_raw certs;
	certificate_raw *certs_ptr;
	certs_ptr=&certs;
	certs_ptr->public_key_pem = "";
	certs_ptr->private_key_pem = "";

	entries.common_name="Github ssl-cert-generator";

	/* generate standalone keys (not signed with other certificate) */
	ssl_gen.create_standalone_keys(&entries, sys_time, &date_end, 509, (char*)"", 2048, &certs);

	logs(Logger::ERR, "public cert: %s", certs_ptr->public_key_pem.c_str());
	logs(Logger::ERR, "private cert: %s", certs_ptr->private_key_pem.c_str());
}

void TestTool::test_string2wstring()
{
	std::string dd = std::string("hello world\0");
	std::wstring wstr = L"";
	Tool::string2wstring(dd, wstr);
	logs_buf_force("wstr", (void*)wstr.c_str(), wstr.length()*2);
}

void TestTool::test_wstring2string()
{
	std::wstring wstr = L"中国";
	std::string dd;
	Tool::wstring2string(wstr, dd);
	logs_buf_force("dd", (void*)dd.c_str(), dd.length());
}

void findSqlKeyWord(std::string src, std::string dst)
{
	logs(Logger::INFO, "ignorecase:: src: %s, dst: %s, pos: %d", src.c_str(), dst.c_str(),
				Tool::find_sqlKeyWord(src, dst, true));

	logs(Logger::INFO, "ignorecase:: src: %s, dst: %s, pos: %d", src.c_str(), dst.c_str(),
					Tool::find_sqlKeyWord(src, dst, false));
}

void TestTool::test_findSqlKeyWord()
{
//	findSqlKeyWord("BEGIN TRANS;select * from bigtable", "BeGin");
//	findSqlKeyWord("BEGINTRANS;select * from bigtable", "BeGin");
//	findSqlKeyWord("BEGIN", "BeGin");
//	findSqlKeyWord("begin", "BeGin");
//	findSqlKeyWord("begin;", "BeGin");
//	findSqlKeyWord("select * from bigtable;begin;", "BeGin");
//	findSqlKeyWord("select * from bigtable;begin;select * from bigtable;", "BeGin");
//	findSqlKeyWord("select * from bigtable;\'begin\';select * from bigtable;", "BeGin");
//	findSqlKeyWord("select * from bigtable;\'begin     dsfsadfsa  \';select * from bigtable;", "BeGin");

	std::string src = "DECLARE @edition sysname SET @edition = cast(SERVERPROPERTY(N ?) as sysname) SELECT CAST(serverproperty(N ?) AS sysname) AS [ Server_Name ] , ? + quotename(CAST(serverproperty(N ?) AS sysname) , ?) + ? AS [ Server_Urn ] , CAST(case when @edition = N ? then ? else ? end AS int) AS [ Server_ServerType ] ,(@@microsoftversion / ? x1000000) & ? xff AS [ VersionMajor ] ,(@@microsoftversion / ? x10000) & ? xff AS [ VersionMinor ] , @@microsoftversion & ? xffff AS [ BuildNumber ] , CAST(SERVERPROPERTY(?) AS bit) AS [ IsSingleUser ] , CAST(SERVERPROPERTY(N ?) AS sysname) AS [ Edition ] , CAST(SERVERPROPERTY(?) AS int) AS [ EngineEdition ] ORDER BY [ Server_Name ] ASC";
	std::string dst = "commit";
	findSqlKeyWord(src, dst);

}

