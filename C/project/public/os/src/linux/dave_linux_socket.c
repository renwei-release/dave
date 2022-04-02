/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include "dave_linux.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "os_log.h"

#define EPOLL_MANAGE_MAX (DAVE_SERVER_SUPPORT_SOCKET_MAX)
#define EPOLL_EDGE_EVENTS (EPOLLIN|EPOLLPRI|EPOLLERR|EPOLLHUP|EPOLLET)	/* EPOLLET is Edge Triggered , not EPOLLET is Level Triggered mode, */
#define EPOLL_LEVEL_EVENTS (EPOLLIN|EPOLLPRI|EPOLLERR|EPOLLHUP)	/* EPOLLET is Edge Triggered , not EPOLLET is Level Triggered mode, */
#define EPOLL_EVENTS EPOLL_EDGE_EVENTS
static void *_linux_socket_epoll_thread = NULL;
static dave_socket_event_fun _epoll_event_notify_fun = NULL;
static int m_epFd;
static struct epoll_event m_pEvent[EPOLL_MANAGE_MAX];
static sb _linux_recv_data_len, _linux_snd_data_len;
static pthread_mutex_t m_epoll_mutex;

typedef struct conn_info {
	int fd;
	int timeout;
} conn_info;

#define EPOLL_CONN_WAIT_MAX (1024)
#define EPOLL_CONN_WAIT_TIMES (3)
static struct conn_info m_connWait[EPOLL_CONN_WAIT_MAX];

static int
_os_linux_domain(SOCDOMAIN domain)
{
	switch(domain)
	{
		case DM_SOC_PF_INET:
		case DM_SOC_PF_RAW_INET:
				return PF_INET;
			break;
		case DM_SOC_PF_INET6:
				return PF_INET6;
			break;
		default:
				return PF_INET;
			break;
	}
}

static int
_os_linux_net_type(SOCTYPE type)
{
	switch(type)
	{
		case TYPE_SOCK_STREAM:
				return SOCK_STREAM;
			break;
		case TYPE_SOCK_DGRAM:
				return SOCK_DGRAM;
			break;
		case TYPE_SOCK_RAW:
				return SOCK_RAW;
			break;
		default:
				return SOCK_STREAM;
			break;
	}	
}

static int
_os_linux_setnonblocking(int sock)
{
	int opts;

	opts = fcntl(sock, F_GETFL);
	if(opts<0)
	{
		return 0;
	}

	opts = opts|O_NONBLOCK ;
	if(fcntl(sock, F_SETFL, opts) < 0)
	{
		return 0;
	}

	return 1;
}

static int
_os_linux_setBlocking(int sock)
{
	int opts;

	opts = fcntl(sock, F_GETFL);
	if(opts<0)
	{
		return 0;
	}

	opts = opts & ~O_NONBLOCK;
	if(fcntl(sock, F_SETFL, opts) < 0 )
	{
		return 0;
	}

	return 1;
}

static void
_os_linux_blockset(int sock, dave_bool flag)
{
	if(flag == dave_false)
	{
		_os_linux_setnonblocking(sock);
	}
	else
	{
		_os_linux_setBlocking(sock);
	}
}

static s32
_os_linux_normal_setup_socket(int linux_type, NetAddrType addr_type, int sock)
{
	int optval;
	socklen_t optval_len;
	struct timeval timeout;

	_os_linux_blockset(sock, dave_false);

	if(linux_type == SOCK_STREAM)
	{
		optval = 1;
	}
	else
	{
		optval = 0;
	}
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)(&optval), sizeof(optval)) == -1)
	{
		close(sock);
		OSDEBUG("set reuseaddr error!!");
		return INVALID_SOCKET_ID;
	}

	if(linux_type == SOCK_DGRAM)
	{
		optval = 1;
		if(setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &optval, sizeof(optval)) == -1)
		{
			OSLOG("setup IPPROTO_IP failed!");
		}
	}

	optval = 1024 * 128;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void *)(&optval), sizeof(optval)) == -1)
	{
		close(sock);
		OSDEBUG("set rcv buf error!!");
		return INVALID_SOCKET_ID;
	}

	optval = 1024 * 64;
	if(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void *)(&optval), sizeof(optval)) == -1)
	{
		close(sock);
		OSDEBUG("set snd buf error!!");
		return INVALID_SOCKET_ID;
	}
	
	optval = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVLOWAT, (void *)(&optval), sizeof(optval)) == -1)
	{
		OSDEBUG("set rcv low error!!");
	}

	optval = 0;
	optval_len = sizeof(optval);
	getsockopt(sock, SOL_SOCKET, SO_SNDLOWAT, (void *)(&optval), &optval_len);

	if(linux_type == SOCK_DGRAM)
	{
		optval = IPTOS_LOWDELAY;
		if(setsockopt(sock, IPPROTO_IP, IP_TOS, (void *)(&optval), sizeof(optval)) == -1)
		{
			OSDEBUG("set tos error!!");
		}		
	}

	if(linux_type == SOCK_STREAM)
	{
		optval = 1024 * 1024 * 8;
		if(setsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, (void *)(&optval), sizeof(optval)) < 0)
		{
			OSDEBUG("set tcp max seg error!!");
		}
	}

	/* Disable the Nagle (TCP No Delay) algorithm */
	if(linux_type == SOCK_STREAM)
	{
		optval = 0;
		if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)(&optval), sizeof(optval)) == -1)
		{
			close(sock);
			OSDEBUG("set Nagle error!!");
			return INVALID_SOCKET_ID;
		}
	}

	// Set Receive and accept timer out.
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

	if(addr_type == NetAddrIPBroadcastType)
	{
		optval = 1;
		if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) == -1)  
		{
			close(sock);
			OSDEBUG("set socket error!");	
			return INVALID_SOCKET_ID;  
		}

	}

	return sock;
}

static s32
_os_linux_keepalive_setup_socket(s32 socket, sb keepalive_times)
{
	int iKeepAlive = 1;
	int keepidle = 5;
	int keepinterval = 5;
	int keepcount = 3;

	if(keepalive_times < 5)
	{
		keepidle = 1;
		keepinterval = 1;
	}

	setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&iKeepAlive, sizeof(iKeepAlive));
	setsockopt(socket, SOL_TCP, TCP_KEEPIDLE, (void *)&keepidle, sizeof(keepidle));
	setsockopt(socket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));
	setsockopt(socket, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));

	return socket;
}

static void
_os_linux_bind_netcard(s32 socket, s8 *netcard_name)
{
	struct ifreq nif;

	if((netcard_name != NULL)
		&& (netcard_name[0] != '\0')
		&& (t_is_all_show_char((u8 *)netcard_name, dave_strlen(netcard_name)) == dave_true))
	{
		strncpy(nif.ifr_name, (char *)netcard_name, dave_strlen(netcard_name) + 1);

		if(setsockopt(socket, SOL_SOCKET, SO_BINDTODEVICE, (char *)&nif, sizeof(nif)) < 0)
		{
			OSABNOR("setup SOL_SOCKET failed! [%d/%s]", socket, nif.ifr_name);
		}
	}
}

static void
_os_linux_epoll_add_event(int fd, __uint32_t event)
{
	if(m_epFd >= 0)
	{
		struct epoll_event ev;

		ev.events = event;
		ev.data.fd = fd;
		epoll_ctl(m_epFd, EPOLL_CTL_ADD, ev.data.fd, &ev);
	}
}

static void
_os_linux_epoll_del_event(int fd)
{
	if(m_epFd >= 0)
	{
		struct epoll_event ev;

		ev.events = 0;
		ev.data.fd = fd;
		epoll_ctl(m_epFd, EPOLL_CTL_DEL, ev.data.fd, &ev);
	}
}

static void
_os_linux_epoll_mdf_event(int fd, __uint32_t event)
{
	if(m_epFd >= 0)
	{
		struct epoll_event ev;

		OSDEBUG("fd:%d event:%x", fd, event);		

		ev.events = event;
		ev.data.fd = fd;
		epoll_ctl(m_epFd, EPOLL_CTL_MOD, ev.data.fd, &ev);
	}
}

static void
_os_linux_epoll_event_notify(SOCEVENT event, sw_int32 socket_id)
{
	dave_bool level_trigger;

	if(event == SOC_EVENT_CONNECT)
	{
		_os_linux_epoll_mdf_event(socket_id, EPOLL_EVENTS);
	}

	if(_epoll_event_notify_fun != NULL)
	{
		if((EPOLL_EVENTS & EPOLLET) == EPOLLET)
			level_trigger = dave_false;
		else
			level_trigger = dave_true;

		_epoll_event_notify_fun(event, socket_id, level_trigger);
	}
}

static void
_os_linux_epoll_reset_wait(void)
{
	ub index;

	for(index=0; index<EPOLL_CONN_WAIT_MAX; index++)
	{
		m_connWait[index].fd = -1;
		m_connWait[index].timeout = -1;
	}
}

static dave_bool
_os_linux_epoll_add_wait(int socket)
{
	ub index;

	pthread_mutex_lock(&m_epoll_mutex);
	for(index=0; index<EPOLL_CONN_WAIT_MAX; index++)
	{
		if(m_connWait[index].fd == -1)
		{
			m_connWait[index].fd = socket;
			m_connWait[index].timeout = EPOLL_CONN_WAIT_TIMES;
			break;
		}
	}
	pthread_mutex_unlock(&m_epoll_mutex);

	if(index >= EPOLL_CONN_WAIT_MAX)
		return dave_false;
	else
		return dave_true;
}

static dave_bool
_os_linux_epoll_del_wait(int socket)
{
	ub index;

	pthread_mutex_lock(&m_epoll_mutex);
	for(index=0; index<EPOLL_CONN_WAIT_MAX; index++)
	{
		if(m_connWait[index].fd == socket)
		{
			m_connWait[index].fd = -1;
			m_connWait[index].timeout = -1;
			break;
		}
	}
	pthread_mutex_unlock(&m_epoll_mutex);

	if(index >= EPOLL_CONN_WAIT_MAX)
		return dave_false;
	else
		return dave_true;	
}

static void
_os_linux_epoll_check_wait(void)
{
	ub index;

	pthread_mutex_lock(&m_epoll_mutex);
	for(index=0; index<EPOLL_CONN_WAIT_MAX; index++)
	{
		if((m_connWait[index].fd != -1) && (m_connWait[index].timeout > 0))
		{
			if((-- m_connWait[index].timeout) <= 0)
			{
				_os_linux_epoll_event_notify(SOC_EVENT_CONNECT_FAIL, m_connWait[index].fd);
				m_connWait[index].fd = -1;
				m_connWait[index].timeout = -1;
			}
		}
	}
	pthread_mutex_unlock(&m_epoll_mutex);
}

static void *
_os_linux_epoll_event_thread(void *arg)
{
	int iMillisecond = 3000;
	int event_index, m_nfds;

	while(dave_os_thread_canceled(_linux_socket_epoll_thread) == dave_false)
	{
		m_nfds = epoll_wait(m_epFd, m_pEvent, EPOLL_MANAGE_MAX, iMillisecond);

		if(m_nfds < 0)
		{
			if(errno != EINTR)
			{
				OSABNOR("epoll unexpected errno(%d/%s)", errno, strerror(errno));
			}
			continue;
		}
		else if(m_nfds == 0)
		{
			_os_linux_epoll_check_wait();
		}

		for(event_index=0; event_index<m_nfds; event_index++)
		{
			OSDEBUG("socket:%d event:%x", m_pEvent[event_index].data.fd, m_pEvent[event_index].events);

			if((m_pEvent[event_index].events) & EPOLLOUT)
			{
				sw_uint32 error = 0;
				sw_uint32 len = sizeof(int);

				if(_os_linux_epoll_del_wait(m_pEvent[event_index].data.fd) == dave_false)
				{
					continue;
				}

				getsockopt(m_pEvent[event_index].data.fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
				if(error == 0)
				{
					_os_linux_epoll_event_notify(SOC_EVENT_CONNECT, m_pEvent[event_index].data.fd);
				}
				else
				{
					_os_linux_epoll_event_notify(SOC_EVENT_CONNECT_FAIL, m_pEvent[event_index].data.fd);
				}
			}
			else if((m_pEvent[event_index].events) & (EPOLLERR|EPOLLHUP))
			{
				_os_linux_epoll_event_notify(SOC_EVENT_CLOSE, m_pEvent[event_index].data.fd);
			}
			else if(((m_pEvent[event_index].events) & EPOLLIN) || ((m_pEvent[event_index].events) & EPOLLPRI))
			{
				_os_linux_epoll_event_notify(SOC_EVENT_REV, m_pEvent[event_index].data.fd);
			}
		}
	}

	dave_os_thread_exit(_linux_socket_epoll_thread);

	_linux_socket_epoll_thread = NULL;

	return NULL;
}

static dave_bool
_os_linux_socket_bind_fix_port(s32 socket, u16 port)
{
	struct sockaddr_in bind_port;

	bind_port.sin_family = AF_INET;
	bind_port.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_port.sin_port = htons(port);
	if(bind(socket, (struct sockaddr*)&bind_port, sizeof(struct sockaddr_in)) < 0)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

static dave_bool
_os_linux_socket_bind_ip_v4(s32 socket, SocNetInfo *pNetInfo)
{
	int bind_result;
	struct sockaddr_in my_addr;
	s8 ip_str[20];

	ipstr(pNetInfo->addr.ip.ip_addr, 4, ip_str, 20);
	dave_memset(&my_addr, 0x00, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(pNetInfo->port);
	if(pNetInfo->addr_type == NetAddrIPBroadcastType)
	{
		my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		my_addr.sin_addr.s_addr = inet_addr((char *)ip_str);
	}

	bind_result = bind(socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

	if(bind_result >= 0)
	{
		OSDEBUG("BIND socket=%d(%s) bind_result=%d port=%d ip_str=%s",
			socket, pNetInfo->type==TYPE_SOCK_STREAM?"TCP":"UDP",
			bind_result, pNetInfo->port, ip_str);
		return dave_true;
	}	
	else
	{
		OSDEBUG("socket bind<port:%d> failed: errno(%d):%s",
			pNetInfo->port, errno, strerror(errno));
		return dave_false;
	}
}

static dave_bool
_os_linux_socket_bind_ip_v6(s32 socket, SocNetInfo *pNetInfo)
{
	return dave_false;
}

// =====================================================================

dave_bool
dave_os_socket_init(dave_socket_event_fun event_call_back)
{
	_epoll_event_notify_fun = event_call_back;

	pthread_mutex_init(&m_epoll_mutex, NULL);

	_os_linux_epoll_reset_wait();

	m_epFd = epoll_create(EPOLL_MANAGE_MAX);
	if(m_epFd < 0)
	{
		OSDEBUG("ERROR !!! creat epoll fail!");
		return dave_false;
	}

	_linux_socket_epoll_thread = dave_os_create_thread("epoll", _os_linux_epoll_event_thread, NULL);
	if(_linux_socket_epoll_thread == NULL)
	{
		OSABNOR("epoll thread failed!");
	}

	_linux_recv_data_len = _linux_snd_data_len = 0;

	return dave_true;
}

dave_bool
dave_os_socket_exit(void)
{
	_epoll_event_notify_fun = NULL;

	if(_linux_socket_epoll_thread != NULL)
	{
		dave_os_release_thread(_linux_socket_epoll_thread);
	}

	pthread_mutex_destroy(&m_epoll_mutex);

	return dave_true;
}

s32
dave_os_socket(SOCDOMAIN domain, SOCTYPE type, NetAddrType addr_type, s8 *netcard_name)
{
	int linux_domain = _os_linux_domain(domain);
	int linux_type = _os_linux_net_type(type);
	int socket_id;
	s8 ifname[128];

	dave_memset(ifname, 0x00, 128);

	socket_id = socket(linux_domain, linux_type, 0);
	if(socket_id < 0)
	{
		OSLOG("errno:%d(%s)", errno, strerror(errno));
		return socket_id;
	}
	else
	{
		if(type == TYPE_SOCK_DGRAM)
		{
			_os_linux_epoll_add_event(socket_id, EPOLL_EVENTS);
		}
	}

	socket_id = _os_linux_normal_setup_socket(linux_type, addr_type, socket_id);

	_os_linux_bind_netcard(socket_id, netcard_name);

	OSDEBUG("domain:%d type:%d addr_type:%d netcard_name:%s socket:%d",
		domain, type, addr_type, netcard_name, socket_id);

	return socket_id;
}

SOCCNTTYPE
dave_os_connect(s32 socket, SocNetInfo *pNetInfo)
{
	struct sockaddr_in addr;
    struct sockaddr_in guest;
    socklen_t guest_len = sizeof(guest);
	s8 ip_str[20];
	int ret;

	if(pNetInfo->fixed_src_flag == FixedPort)
	{
		if(_os_linux_socket_bind_fix_port(socket, pNetInfo->src_port) == dave_false)
		{
			OSLOG("socket:%d bind fix port:%d failed!", socket, pNetInfo->src_port);
			return SOC_CNT_FAIL;
		}
		else
		{
			OSDEBUG("socket:%d bind fix port:%d ok!", socket, pNetInfo->src_port);
		}
	}

	ipstr(pNetInfo->addr.ip.ip_addr, 4, ip_str, 20);
	dave_memset(&addr, 0x00, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pNetInfo->port);
	addr.sin_addr.s_addr = inet_addr((char *)ip_str);

	if(pNetInfo->enable_keepalive_flag == KeepAlive_enable)
	{
		OSDEBUG("socket:%d enable keepalive!", socket);
		_os_linux_keepalive_setup_socket(socket, pNetInfo->keepalive_second);
	}

	ret = connect(socket, (struct sockaddr *)&addr, sizeof(addr));

	OSDEBUG("socket:%d %s ret:%d", socket, ipv4str(pNetInfo->addr.ip.ip_addr, pNetInfo->port), ret);

	if(ret < 0)
	{
		if(errno != EINPROGRESS)
		{
			OSLOG("errno:%d(%s)", errno, strerror(errno));
			return SOC_CNT_FAIL;
		}
	}

	getsockname(socket, (struct sockaddr *)&guest, &guest_len);
	pNetInfo->src_port = ntohs(guest.sin_port);

	if(m_epFd >= 0)
	{
		if(_os_linux_epoll_add_wait(socket) == dave_false)
		{
			OSABNOR("too many wait connection!");
			return SOC_CNT_FAIL;
		}
		else
		{
			_os_linux_epoll_add_event(socket, EPOLL_EVENTS|EPOLLOUT);

			if(ret < 0)
				return SOC_CNT_WAIT;
			else
				return SOC_CNT_OK;
		}
	}
	else
	{
		OSABNOR("error m_epFd:%d", m_epFd);
		return SOC_CNT_FAIL;
	}
}

dave_bool
dave_os_bind(s32 socket, SocNetInfo *pNetInfo)
{
	dave_bool bind_v4_ret, bind_v6_ret;

	bind_v4_ret = _os_linux_socket_bind_ip_v4(socket, pNetInfo);
	bind_v6_ret = _os_linux_socket_bind_ip_v6(socket, pNetInfo);

	if((bind_v4_ret == dave_true) || (bind_v6_ret == dave_true))
	{
		return dave_true;
	}
	else
	{
		OSDEBUG("errno:%d(%s)", errno, strerror(errno));
		return dave_false;
	}
}

dave_bool
dave_os_listen(sw_int32 socket, sw_uint32 backlog)
{
	if(listen(socket, backlog) >= 0)
	{
		_os_linux_epoll_add_event(socket, EPOLL_EVENTS);

		OSDEBUG("socket:%d", socket);

		return dave_true;
	}
	else
	{
		OSABNOR("socket listen failed: errno(%d):%s", errno, strerror(errno));
		return dave_false;
	}
}

s32
dave_os_accept(s32 socket, SocNetInfo *pNetInfo)
{
	int linux_type = _os_linux_net_type(pNetInfo->type);
	sw_int32 child_socket;
	struct sockaddr_in guest;
	socklen_t guest_len = sizeof(guest);

	child_socket = accept(socket, NULL, NULL);
	if(child_socket < 0)
	{
		if(errno != 24)
		{
			OSDEBUG("socket:%d accept fail! child_socket=%d errno(%d):%s",
				socket, child_socket, errno, strerror(errno));
		}
		else
		{
			OSABNOR("socket:%d accept fail! child_socket=%d errno(%d):%s",
				socket, child_socket, errno, strerror(errno));
		}
		return child_socket;
	}

	getpeername(child_socket, (struct sockaddr *)&guest, &guest_len);

	strip((s8 *)inet_ntoa(guest.sin_addr), dave_strlen((s8 *)inet_ntoa(guest.sin_addr)), pNetInfo->addr.ip.ip_addr, 16);
	pNetInfo->port = ntohs(guest.sin_port);

	OSDEBUG("accept socket=%d/%d %s",
		child_socket, socket,
		ipv4str(pNetInfo->addr.ip.ip_addr, pNetInfo->port));

	if(pNetInfo->enable_keepalive_flag == KeepAlive_enable)
	{
		OSDEBUG("socket:%d enable keepalive!", child_socket);
		_os_linux_keepalive_setup_socket(child_socket, pNetInfo->keepalive_second);
	}

	return _os_linux_normal_setup_socket(linux_type, pNetInfo->addr_type, child_socket);
}

void
dave_os_epoll(s32 socket)
{
	_os_linux_epoll_add_event(socket, EPOLL_EVENTS);
}

dave_bool
dave_os_recv(s32 socket, SocNetInfo *pNetInfo, u8 *data, ub *data_len)
{
	s32 recv_len;
	struct sockaddr_in remote_addr;
	socklen_t remote_addrlen = sizeof(remote_addr);

	dave_memset(&remote_addr, 0x00, sizeof(remote_addr));

	recv_len = recvfrom(socket, data, *data_len, 0, (struct sockaddr *)(&remote_addr), &remote_addrlen);

	if(recv_len == 0)
	{
		*data_len = 0;
		if(pNetInfo->type == TYPE_SOCK_DGRAM)
		{
			return dave_true;
		}
		OSDEBUG("socket:%d errno:%d/%s", socket, errno, strerror(errno));
		return dave_false;
	}
	else if(recv_len < 0)
	{
		*data_len = 0;
		if(pNetInfo->type == TYPE_SOCK_DGRAM)
		{
			return dave_true;
		}
		else
		{
			switch(errno)
			{
				case 0:				// Success
					return dave_true;
				case 9:				// Bad file descriptor
					return dave_false;
				case 11:			// Resource temporarily unavailable
					return dave_true;
				case 88:			// Socket operation on non-socket
				case 104:			// Connection reset by peer
				case 107:			// Transport endpoint is not connected
						return dave_false;
					break;
				case 115:			// Operation now in progress
						return dave_true;
					break;
				default:
						OSLOG("socket=%d rev fail! code=%d errno(%d)=%s", socket, recv_len, errno, strerror(errno));
						return dave_false;
					break;
			}
		}
	}
	else
	{
		pNetInfo->src_port = ntohs(remote_addr.sin_port);
		strip((s8 *)inet_ntoa(remote_addr.sin_addr), 20, pNetInfo->src_ip.ip_addr, 4);

		*data_len = (ub)recv_len;
		_linux_recv_data_len += (sb)recv_len;
		return dave_true;
	}
}

sb
dave_os_send(s32 socket, SocNetInfo *pNetInfo, u8 *data, ub data_len, dave_bool urg)
{
	sb snd_len;
	struct sockaddr_in addr;
	s8 ip_str[20];
	unsigned int flags;

	if(socket < 0)
	{
		return -1;
	}

	if(pNetInfo->type == TYPE_SOCK_STREAM)
	{
		if(urg == dave_true)
		{
			flags = MSG_DONTWAIT | MSG_OOB;
		}
		else
		{
			flags = MSG_DONTWAIT;
		}

		snd_len = send(socket, data, data_len, flags);
		if (snd_len >= 0)
		{
			_linux_snd_data_len += (sb)snd_len;
			return snd_len;
		}
		else
		{
			if (errno == EAGAIN)
			{
				return 0;
			}
			else
			{					
				if(errno != EPIPE)
				{
					OSDEBUG("socket:%d snd_len:%d data_len:%d ret:%d errno:%s",
						socket, snd_len, data_len, snd_len, strerror(errno));
				}
				return snd_len;
			}
		}
	}
	else if(pNetInfo->type == TYPE_SOCK_DGRAM)
	{
		ipstr(pNetInfo->addr.ip.ip_addr, 4, ip_str, 20);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(pNetInfo->port);
		if(pNetInfo->addr_type == NetAddrIPBroadcastType)
		{
			addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		}
		else
		{
			addr.sin_addr.s_addr = inet_addr((char *)ip_str);
		}

		snd_len = sendto(socket, data, data_len, 0, (struct sockaddr *)(&addr), sizeof(struct sockaddr_in));
		if (snd_len >= 0)
		{
			_linux_snd_data_len += (sb)snd_len;
			return snd_len;
		}
		else
		{
			if(errno == EAGAIN)
			{
				return 0;
			}
			else
			{
				return snd_len;
			}
		}
	}
	else
	{
		OSDEBUG("socket send type:%d", pNetInfo->type);
		return 0;
	}
}

dave_bool
dave_os_close(s32 socket, dave_bool clean_wait)
{
	OSDEBUG("socket=%d", socket);

	if(socket >= 0)
	{
		if(clean_wait == dave_true)
		{
			_os_linux_epoll_del_wait(socket);
		}

		_os_linux_epoll_del_event(socket);

		shutdown(socket, SHUT_RDWR);
		close(socket);
	}

	return dave_true;
}

