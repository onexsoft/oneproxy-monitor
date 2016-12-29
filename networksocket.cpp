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
* @ClassName: networksocket.cpp
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月29日
*
*/

#include "systemapi.h"
#include "networksocket.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int NetworkSocket::addr_pton()
{
	memset(&this->m_addr, 0, sizeof(this->m_addr));
	if (this->m_address.compare("unix") == 0)
	{
		this->set_addr(AF_UNIX, this->get_port());
	}
	else if (this->m_address.compare("*") == 0)
	{
		this->set_addr(AF_INET, this->get_port());
		this->m_addr.sin.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else if (this->m_address.find(":") != this->m_address.npos)
	{
		this->set_addr(AF_INET6, this->get_port());
		if (SystemApi::system_inetPton(AF_INET6, this->m_address.c_str(), &this->m_addr.sin6.sin6_addr) < 0) {
			logs(Logger::FATAL, "inet_pton error, address: %s", this->m_address.c_str());
			return -1;
		}
	}
	else
	{
		this->set_addr(AF_INET, this->get_port());
		if (SystemApi::system_inetPton(AF_INET, this->m_address.c_str(), &this->m_addr.sin.sin_addr) < 0) {
			logs(Logger::FATAL, "inet_pton error, address: %s", this->m_address.c_str());
			return -1;
		}
	}
	return 0;
}

int NetworkSocket::addr_ntop()
{
	char buf[INET6_ADDRSTRLEN + 10];
	int port = 0;

	memset(buf, 0, sizeof(buf));
	this->m_address.clear();

	switch(this->m_addr.sa.sa_family) {
		case AF_UNIX:
			this->m_address = "unix";
			break;
		case AF_INET:
		{
			logs(Logger::INFO, " CURRENT IS IPV4");
			if (SystemApi::system_ntop(AF_INET, &this->m_addr.sin.sin_addr, buf, sizeof(buf)) < 0) {
				logs(Logger::ERR, "inet_ntop error");
			}
			port = this->m_addr.sin.sin_port;
		}
			break;
		case AF_INET6:
		{
			if (SystemApi::system_ntop(AF_INET6, &this->m_addr.sin6.sin6_addr, buf, sizeof(buf)) < 0) {
				logs(Logger::ERR, "inet_ntop error");
			}
			port = this->m_addr.sin6.sin6_port;
		}
			break;
		default:
			logs(Logger::ERR, "unkown address");
	}
	if (strlen(buf) > 0) {
		this->m_address = std::string(buf);
		this->m_port = port;
	}

	return 0;
}

int NetworkSocket::addr_assign(const sockaddr *addr)
{
	switch (addr->sa_family) {
		case AF_INET:
			memcpy(&this->m_addr.sin, addr, sizeof(this->m_addr.sin));
			break;
		case AF_INET6:
			memcpy(&this->m_addr.sin6, addr, sizeof(this->m_addr.sin6));
			break;
		case AF_UNIX:
		{
			logs(Logger::INFO, "AF_UNIX copy not supported");
			return -1;
		}
	}

	return 0;
}

int NetworkSocket::is_validAddress()
{
	if (this->get_port() > 0 && this->get_address().size() > 0)
		return 1;
	else
		return 0;
}

int NetworkSocket::set_sockReUseAddr(unsigned int sfd)
{
	int val = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val)) < 0) {
		logs(Logger::ERR, "set sock(%d) resueaddr error, errmsg:%s", sfd, SystemApi::system_strerror());
		return -1;
	}
	return 0;
}

int NetworkSocket::set_sockReUsePort(unsigned int sfd)
{
#ifndef _WIN32
	int val = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&val, sizeof(val)) < 0) {
		logs(Logger::ERR, "set sock(%d) resueport error, errmsg: %s", sfd, SystemApi::system_strerror());
		return -1;
	}
#endif
	return 0;
}

int NetworkSocket::set_sockCommonOpt(unsigned int sfd, int is_unix)
{
	int val;

#ifndef WIN32
	/* close fd on exec */
	if(fcntl(sfd, F_SETFD, FD_CLOEXEC) < 0)
	{
		logs(Logger::ERR, "set sock(%d) cloexec flag error", sfd);
		return -1;
	}
#endif

	/* when no data available, return EAGAIN instead blocking */
	if (SystemApi::system_setSockNonblocking(sfd, false) < 0 ) {
		logs(Logger::ERR, "set non blocking error");
		return -1;
	}

	/*
	 * Following options are for network sockets
	 */
	if (is_unix)
		return 0;

	/* the keepalive stuff needs some poking before enbling */

	/* turn on socket keepalive */
	val = 1;
	if(setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (char*)&val, sizeof(val)) < 0) {
		logs(Logger::ERR, "set socket keepalive error(%s)", SystemApi::system_strerror());
		return -1;
	}

#ifdef __linux__
	/* set count of keepalive packets */
	if (cf_tcp_keepcnt > 0) {
		val = cf_tcp_keepcnt;
		if (setsockopt(sfd, IPPROTO_TCP, TCP_KEEPCNT, (char*)&val, sizeof(val)) < 0) {
			logs(Logger::ERR, "set socket keepcnt error(%s)", SystemApi::system_strerror());
			return -1;
		}
	}

	/* how long the connection can stay idle before sending keepalive pkts */
	if (cf_tcp_keepidle) {
		val = cf_tcp_keepidle;
		if (setsockopt(sfd, IPPROTO_TCP, TCP_KEEPIDLE, (char*)&val, sizeof(val)) < 0) {
			logs(Logger::ERR, "set socket keepidle error(%s)", SystemApi::system_strerror());
			return -1;
		}
	}
	/* time between packets */
	if (cf_tcp_keepintvl) {
		val = cf_tcp_keepintvl;
		if(setsockopt(sfd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
			logs(Logger::ERR, "set socket keepintvl error(%s)", SystemApi::system_strerror());
			return -1;
		}
	}
#else
#ifdef TCP_KEEPALIVE
	if (cf_tcp_keepidle) {
		val = cf_tcp_keepidle;
		if(setsockopt(sfd, IPPROTO_TCP, TCP_KEEPALIVE, &val, sizeof(val)) < 0) {
			logs(Logger::ERR, "set socket keepalive error(%s)", SystemApi::system_strerror());
			return -1;
		}
	}
#endif
#endif

	/* set in-kernel socket buffer size */
	if (cf_tcp_socket_buffer) {
		val = cf_tcp_socket_buffer;
		if(setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, (char*)&val, sizeof(val)) < 0) {
			logs(Logger::ERR, "set socket sndbuf error(%s)", SystemApi::system_strerror());
			return -1;
		}

		val = cf_tcp_socket_buffer;
		if(setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (char*)&val, sizeof(val)) < 0) {
			logs(Logger::ERR, "set socket rcvbuf error(%s)", SystemApi::system_strerror());
			return -1;
		}
	}

	/*
	 * Turn off kernel buffering, each send() will be one packet.
	 */
	val = 1;
	if(setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(val)) < 0) {
		logs(Logger::ERR, "set socket nodelay error(%s)", SystemApi::system_strerror());
		return -1;
	}
	return 0;
}

int NetworkSocket::parse_address()
{
	//1. judge the address is valid or not
	if (!this->is_validAddress()) {
		logs(Logger::ERR, "the port(%d) or address(%s) is not valid", this->get_port(), this->get_address().c_str());
		return -1;
	}

	struct addrinfo *ai, *gaires = NULL;
	//2.parse address
	{
		char service[64];
		snprintf(service, sizeof(service), "%d", this->get_port());

		struct addrinfo hints;
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		if (SystemApi::getaddrinfo(this->get_address().c_str(), service, &hints, &gaires) < 0) {
			logs(Logger::FATAL, "call getaddrinfo error, address(%s)", this->get_address().c_str());
			return -1;
		}
	}

	//3. SAVE the addr.
	for(ai = gaires; ai != NULL; ai = ai->ai_next) {
		if (ai->ai_family == AF_UNIX) {
			this->set_addr(ai->ai_family, this->get_port());
		} else {
			this->addr_assign(ai->ai_addr);
		}
		break;
	}

	SystemApi::freeaddrinfo(gaires);

	return 0;
}

void NetworkSocket::set_addr(int af, unsigned int port)
{
	if (af == AF_INET6) {
		this->m_addr.sin6.sin6_family = af;
		this->m_addr.sin6.sin6_port = htons(port);
	} else {
		this->m_addr.sin.sin_family = af;
		this->m_addr.sin.sin_port = htons(port);
	}
}

void NetworkSocket::set_portAndAddr(unsigned int port, std::string address) {
	set_port(port);
	set_address(address);
}

int NetworkSocket::read_data()
{
	unsigned int dataLen = 0;
	if (system_ioctl(this->m_fd, FIONREAD, (unsigned long int*)&dataLen)) {//fd maybe closed
		logs(Logger::ERR, "fd(%d) ioctl FIONREAD error", this->m_fd);
		return -1;
	}

	if (dataLen == ((unsigned int)0)) {
		logs(Logger::DEBUG, "the socket(%d) is closed", this->m_fd);
		return 2;
	}

	this->m_recvData.reallocMem(this->m_recvData.get_length() + dataLen);
	int len = recv(this->m_fd, (char*)(this->m_recvData.addr() + this->m_recvData.length()), dataLen, 0);
	if (len == -1) {
		errno = SystemApi::system_errno();
		if (errno == EAGAIN || errno == EINTR) {
			return 1;
		}
		logs(Logger::ERR, "recv data from fd(%d) error(%d:%s)",
				this->m_fd, errno, SystemApi::system_strerror(errno));
		return -1;
	}
	this->m_recvData.set_length(this->m_recvData.length() + len);
	return 0;
}

int NetworkSocket::read_dataonBlock()
{
	unsigned int dataLen = 0;
	while(true) {
		if (system_ioctl(this->m_fd, FIONREAD, (unsigned long int*)&dataLen)) {//fd maybe closed
			logs(Logger::ERR, "fd(%d) ioctl FIONREAD error", this->m_fd);
			return -1;
		}
		if (dataLen <= 0) {
			SystemApi::system_sleep(50);
			continue;
		}

		this->m_recvData.reallocMem(this->m_recvData.get_length() + dataLen);
		int len = recv(this->m_fd, (char*)(this->m_recvData.addr() + this->m_recvData.length()), dataLen, 0);
		if (len == -1) {
			errno = SystemApi::system_errno();
			if (errno == EAGAIN || errno == EINTR) {
				SystemApi::system_sleep(50);
				continue;
			}
			logs(Logger::ERR, "recv data from fd(%d) error(%d:%s)",
					this->m_fd, errno, SystemApi::system_strerror(errno));
			return -1;
		}
		this->m_recvData.set_length(this->m_recvData.length() + len);
		break;
	}
	return 0;
}

int NetworkSocket::write_data(StringBuf& buf)
{
	uif (buf.get_remailLength() <= 0) {
		return 0;
	}

	logs_buf("xxx write_data", (char*)(buf.addr() + buf.get_offset()), buf.get_remailLength());
	int len = 0;
	while(buf.get_remailLength() > 0) {
//		logs(Logger::ERR, "send data(%d) bytes to client fd(%d)", buf.get_remailLength(), this->m_fd);
		len = send(this->m_fd, (char*)(buf.addr() + buf.get_offset()), buf.get_remailLength(), 0);
		if (len < 0) {
			errno = SystemApi::system_errno();

#ifdef _WIN32
			if (errno == WSAEWOULDBLOCK) {
				return 1;//retry;
			}
#else
			if (errno == EAGAIN) {
				logs(Logger::ERR, "retry fd: %d", this->m_fd);
				return 1;//retry
			}
#endif
//			logs_buf_force("xxx write_data", (char*)(buf.addr() + buf.get_offset()), buf.get_remailLength());
			logs(Logger::ERR, "send data error, fd(%d), errno: %d, err(%s)",
					this->m_fd, errno, SystemApi::system_strerror(errno));
			return -1;
		}
		buf.set_offset(buf.get_offset() + len);
	}
	return 0;
}

void NetworkSocket::save_data(StringBuf& buf)
{
	//first free some memory and then copy data to sendData
	this->m_sendData.erase(0, this->m_sendData.get_offset());
	this->m_sendData.set_offset(0);
	this->m_sendData.append((char*)(buf.addr() + buf.get_offset()), buf.get_remailLength());
}

void NetworkSocket::closeSocket(unsigned int fd)
{
	SystemApi::close(fd);
}

void NetworkSocket::clear_dataBuf()
{
	this->m_recvData.clear();
	this->clear_sendData();
}

void NetworkSocket::clear_sendData()
{
	this->m_sendData.clear();
}

std::string NetworkSocket::connArgsMap2String()
{
	StringBuf result;
	KVStringMap::iterator it = this->m_connArgsMap.begin();
	for(; it != this->m_connArgsMap.end(); ++it) {
		result.appendFormat("%s:%s;", it->first.c_str(), it->second.c_str());
	}
	return std::string(result.addr(), result.length());
}

void NetworkSocket::add_connArgs(const std::string& key, const std::string& value)
{
	this->m_connArgsMap[key] = value;
}

void NetworkSocket::add_backendArgs(const std::string& key, const std::string& value)
{
	this->m_backendArgsMap[key] = value;
}
