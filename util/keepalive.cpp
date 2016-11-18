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
* @FileName: keepalive.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年9月8日
*
*/

#include "keepalive.h"
#include "logger.h"
#include "systemapi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifdef linux
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#endif

bool KeepAlive::stop_keepAlive = false;
KeepAlive::KeepAlive() {
	// TODO Auto-generated constructor stub

}

KeepAlive::~KeepAlive() {
	// TODO Auto-generated destructor stub
}

//return 1: represent in child, -1: error, 0: exit normal.
int KeepAlive::keepalive(int *child_exit_status)
{
#ifndef _WIN32
	int nprocs = 0;
	pid_t child_pid = -1;

	while(KeepAlive::stop_keepAlive == false) {
		/* try to start the children */
		while (nprocs < 1) {
			pid_t pid = fork();

			if (pid == 0) {
				/* child */
				logs(Logger::DEBUG, "we are the child: %d", getpid());
				return 1;
			} else if (pid < 0) {
				/* fork() failed */
				logs(Logger::FATAL, "fork() failed(%s)", SystemApi::system_strerror());
				return -1;
			} else {
				/* we are the angel, let's see what the child did */
				logs(Logger::DEBUG, "[angel] we try to keep PID=%d alive", pid);

				/* forward a few signals that are sent to us to the child instead */
				signal(SIGINT, KeepAlive::handle_signal);
				signal(SIGTERM, KeepAlive::handle_signal);
				signal(SIGHUP, KeepAlive::handle_signal);
				signal(SIGUSR1, KeepAlive::handle_signal);
				signal(SIGUSR2, KeepAlive::handle_signal);

				child_pid = pid;
				nprocs++;
			}
		}

		if (child_pid != -1) {
			struct rusage tRusage;
			int exit_status;
			pid_t exit_pid;

			logs(Logger::DEBUG, "waiting for %d", child_pid);
			memset(&tRusage, 0, sizeof(tRusage)); /* make sure everything is zero'ed out */
			exit_pid = waitpid(child_pid, &exit_status, 0);

			logs(Logger::DEBUG, "%d returned: %d", child_pid, exit_pid);

			if (exit_pid == child_pid) {//child process, exited.
				/* our child returned, let's see how it went */
				if (WIFEXITED(exit_status)) {//child process is normal exit? if normal exit, return value != 0

					//get child process exit value.
					if (child_exit_status)
						*child_exit_status = WEXITSTATUS(exit_status);

					if (*child_exit_status != 2 && *child_exit_status != 3) {
						logs(Logger::DEBUG, "[angel] PID=%d exited normally with exit-code = %d (it used %ld kBytes max)",
								child_pid, WEXITSTATUS(exit_status), tRusage.ru_maxrss / 1024);

						if (*child_exit_status != 4)
						{
                            signal(SIGINT, SIG_DFL);
                            signal(SIGTERM, SIG_DFL);
                            signal(SIGHUP, SIG_DFL);
                            int time_towait = 2;
                            SystemApi::system_sleep(time_towait);
                            nprocs--;
                            child_pid = -1;
						}
						else
						{
							return 0;
						}
					}
					else
					{
						if (*child_exit_status == 2)
							logs(Logger::DEBUG, "[angel] PID=%d died on yy_fatal_error ... waiting 2sec before restart", child_pid);
						else if (*child_exit_status == 3)
							logs(Logger::DEBUG, "%s: [angel] PID=%d reboot on admin remand ... waiting 2sec before restart", child_pid);

						signal(SIGINT, SIG_DFL);
						signal(SIGTERM, SIG_DFL);
						signal(SIGHUP, SIG_DFL);

						int time_towait = 2;
						SystemApi::system_sleep(time_towait);

						nprocs--;
						child_pid = -1;
					}
				} else if (WIFSIGNALED(exit_status)) {
					int time_towait = 2;
					/* our child died on a signal
					 *
					 * log it and restart */

					logs(Logger::DEBUG, "[angel] PID=%d died on signal=%d (it used %ld kBytes max) ... waiting 3min before restart",
							child_pid, WTERMSIG(exit_status), tRusage.ru_maxrss / 1024);

					/**
					 * to make sure we don't loop as fast as we can, sleep a bit between
					 * restarts
					 */

					signal(SIGINT, SIG_DFL);
					signal(SIGTERM, SIG_DFL);
					signal(SIGHUP, SIG_DFL);

					SystemApi::system_sleep(time_towait);

					nprocs--;
					child_pid = -1;
				} else if (WIFSTOPPED(exit_status)) {
				} else {
					logs(Logger::DEBUG, "not reach...");
				}
			} else if (-1 == exit_pid) {
				/* EINTR is ok, all others bad */
				if (EINTR != errno) {
					/* how can this happen ? */
					logs(Logger::ERR, "wait4(%d, ...) failed: %s", child_pid, SystemApi::system_strerror());
					return -1;
				}
			}
		}
	}
#endif
	return 0;
}

void KeepAlive::handle_signal(int sig)
{
	logs(Logger::DEBUG, "handle signal sig: %d", sig);
#ifdef _WIN32
	return;
#else
	signal(sig, SIG_IGN);//忽略
	if (sig == SIGUSR1) {
		kill(0, SIGINT);//发送信号停止子进程
		KeepAlive::stop_keepAlive = true;
	} else {
		kill(0, sig);
	}
#endif
}
