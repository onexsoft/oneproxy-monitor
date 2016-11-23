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
 * @FileName: PidManager.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2016年11月17日 下午2:50:50
 *  
 */

#ifndef PIDMANAGER_H_
#define PIDMANAGER_H_

#include <sstream>
#include "tool.h"
#include "systemapi.h"
#include "config.h"

class PidManager{
public:
	static int kill_process(const char* pid_file)
	{
#ifndef _WIN32
		assert(pid_file != NULL);

		FILE* file = fopen(pid_file, "r");
		if (file == NULL) {
			logs(Logger::ERR, "fopen %s error(%d:%s)", pid_file,
					SystemApi::system_errno(), SystemApi::system_strerror());
			return -1;
		}

		pid_t oldProcessId = 0;
		fscanf(file, "%d", &oldProcessId);
		logs(Logger::DEBUG, "oldProcessId: %d", oldProcessId);

		//send signal to old process
		if (oldProcessId > 0)
			kill(oldProcessId, SIGUSR1);

		fclose(file);
#endif
		return 0;
	}

	static int handle_reboot(const char* pid_file)
	{
#ifndef _WIN32
		assert(pid_file != NULL);
		std::string dirPath;
		std::string suffix;
		std::string pidFile = std::string(pid_file);

		int index = pidFile.find_last_of('/');
		if (index == -1) {
			dirPath = std::string("./");
			suffix = pidFile;
		} else {
			dirPath = pidFile.substr(0, index);
			suffix = pidFile.substr(index, -1);
		}
		logs(Logger::DEBUG, "desPath: %s, suffix: %s", dirPath.c_str(), suffix.c_str());
		pidFile = Tool::search_oneFile(dirPath, suffix);

		if (pidFile.size() > 0)
			kill_process(pidFile.c_str());
#endif
		return 0;
	}

	static int save_pid(const char* pid_file) {
#ifndef _WIN32
		assert(pid_file != NULL);
		std::stringstream currentProcessId;
		if (config()->get_keepAlive()) {
			currentProcessId << getppid();
		} else {
			currentProcessId << getpid();
		}

		std::stringstream ss;
		ss << currentProcessId.str().c_str() << "." << pid_file;

		FILE* file = fopen(ss.str().c_str(), "w");
		if (file == NULL) {
			logs(Logger::ERR, "fopen %s error(%s)", pid_file, SystemApi::system_strerror());
			return -1;
		}

		if (fwrite((void*)currentProcessId.str().c_str(), currentProcessId.str().length(), 1, file) != 1) {
			logs(Logger::ERR, "write process id error(%s)", SystemApi::system_strerror());
			fclose(file);
			return -1;
		}
		fclose(file);
#endif
		return 0;
	}

	static int unlink_pid(const char* pid_file) {
#ifndef _WIN32
		assert(pid_file != NULL);
		std::stringstream pidFile;
		if (config()->get_keepAlive()) {
			pidFile << getppid() << "." << pid_file;
		} else {
			pidFile << getpid() << "." << pid_file;
		}
		logs(Logger::DEBUG, "unlink_pid: %s", pidFile.str().c_str());
		if (unlink(pidFile.str().c_str())) {
			logs(Logger::ERR, "unlink error(%s)", SystemApi::system_strerror());
		}
#endif
		return 0;
	}
};
#endif /* PIDMANAGER_H_ */
