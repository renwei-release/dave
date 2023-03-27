/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_CYGWIN__
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "os_log.h"

static dave_socket_event_fun _cygwin_event_notify_fun = NULL;
static void *_cygwin_socket_event_thread = NULL;
static pthread_mutex_t m_event_mutex;
static WSAEVENT _eventArray[WSA_MAXIMUM_WAIT_EVENTS];
static SOCKET _sockArray[WSA_MAXIMUM_WAIT_EVENTS];

static int
_os_cygwin_domain(SOCDOMAIN domain)
{
	switch(domain)
	{
		case DM_SOC_PF_INET:
		case DM_SOC_PF_RAW_INET:
				return AF_INET;
			break;
		case DM_SOC_PF_INET6:
				return AF_INET6;
			break;
		default:
				return AF_UNIX;
			break;
	}
}

static int
_os_cygwin_net_type(SOCTYPE type)
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

static void
_os_cygwin_event_reset(void)
{
	ub index;

	for(index=0; index<WSA_MAXIMUM_WAIT_EVENTS; index++)
	{
		_eventArray[index] = NULL;
		_sockArray[index] = INVALID_SOCKET;
	}
}

static dave_bool
_os_cygwin_add_event(int sock)
{
	ub index;
	WSAEVENT event = WSACreateEvent();

	WSAEventSelect(sock, event, FD_READ|FD_ACCEPT|FD_CONNECT|FD_CLOSE);

	pthread_mutex_lock(&m_event_mutex);
	for(index=0; index<WSA_MAXIMUM_WAIT_EVENTS; index++)
	{
		if(_sockArray[index] == INVALID_SOCKET)
		{
			_eventArray[index] = event;
			_sockArray[index] = (SOCKET)sock;
			break;
		}
	}
	pthread_mutex_unlock(&m_event_mutex);

	if(index >= WSA_MAXIMUM_WAIT_EVENTS)
	{
		OSABNOR("can't add event of socket:%d", sock);
		return dave_false;
	}

	OSDEBUG("socket:%d", sock);

	dave_os_thread_wakeup(_cygwin_socket_event_thread);
	return dave_true;
}

static dave_bool
_os_cygwin_del_event(int sock)
{
	ub index;

	pthread_mutex_lock(&m_event_mutex);
	for(index=0; index<WSA_MAXIMUM_WAIT_EVENTS; index++)
	{
		if(_sockArray[index] == sock)
		{
			WSACloseEvent(_eventArray[index]);
			_eventArray[index] = NULL;
			_sockArray[index] = INVALID_SOCKET;
			break;
		}
	}
	pthread_mutex_unlock(&m_event_mutex);

	if(index >= WSA_MAXIMUM_WAIT_EVENTS)
	{
		OSLOG("can't find the socket:%d", sock);
		return dave_false;
	}

	OSDEBUG("socket:%d", sock);

	dave_os_thread_wakeup(_cygwin_socket_event_thread);
	return dave_true;	
}

static void
_os_cygwin_notify_event(SOCEVENT event, int sock)
{
	if(_cygwin_event_notify_fun != NULL)
	{
		OSDEBUG("socket:%d event:%s", sock, t_auto_SOCEVENT_str(event));
		_cygwin_event_notify_fun(event, sock, dave_false);
	}
}

static int
_os_cygwin_load_event_array(WSAEVENT *pEventArray, SOCKET *pSockArray)
{
	ub local_index, load_index;

	load_index = 0;

	pthread_mutex_lock(&m_event_mutex);
	for(local_index=0; local_index<WSA_MAXIMUM_WAIT_EVENTS; local_index++)
	{
		if(_eventArray[local_index] != NULL)
		{
			pEventArray[load_index] = _eventArray[local_index];
			pSockArray[load_index] = _sockArray[local_index];

			load_index ++;
		}
	}
	pthread_mutex_unlock(&m_event_mutex);

	return load_index;
}

static dave_bool
_os_cygwin_socket_bind_fix_port(int sock, u16 port)
{
	struct sockaddr_in bind_port;

	bind_port.sin_family = AF_INET;
	bind_port.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_port.sin_port = htons(port);
	if(bind(sock, (struct sockaddr*)&bind_port, sizeof(struct sockaddr_in)) < 0)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

static int
_os_cygwin_keepalive_setup_socket(int sock, sb keepalive_times)
{
	int iKeepAlive = 1;

	setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&iKeepAlive, sizeof(iKeepAlive));

	return sock;
}

static dave_bool
_os_cygwin_socket_bind_ip_v4(int sock, SocNetInfo *pNetInfo)
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

	bind_result = bind(sock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

	if(bind_result >= 0)
	{
		OSDEBUG("BIND socket=%d(%s) bind_result=%d port=%d ip_str=%s",
			sock, pNetInfo->type==TYPE_SOCK_STREAM?"TCP":"UDP",
			bind_result, pNetInfo->port, ip_str);
		return dave_true;
	}	
	else
	{
		OSDEBUG("socket bind<port:%d> failed: errno(%d)",
			pNetInfo->port, WSAGetLastError());
		return dave_false;
	}
}

static dave_bool
_os_cygwin_socket_bind_ip_v6(int sock, SocNetInfo *pNetInfo)
{
	return dave_false;
}

static int
_os_cygwin_setnonblocking(int sock)
{
	int opts;

	opts = fcntl(sock, F_GETFL);
	if(opts<0)
	{
		return 0;
	}

	opts = opts|O_NONBLOCK;
	if(fcntl(sock, F_SETFL, opts) < 0)
	{
		return 0;
	}

	return 1;
}

static int
_os_cygwin_setBlocking(int sock)
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
_os_cygwin_blockset(int sock, dave_bool flag)
{
	if(flag == dave_false)
	{
		_os_cygwin_setnonblocking(sock);
	}
	else
	{
		_os_cygwin_setBlocking(sock);
	}
}

static s32
_os_cygwin_normal_setup_socket(int linux_type, NetAddrType addr_type, int sock)
{
	int optval;
	socklen_t optval_len;
	struct timeval timeout;

	_os_cygwin_blockset(sock, dave_false);

	if(linux_type == SOCK_STREAM)
	{
		optval = 1;
	}
	else
	{
		optval = 0;
	}
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)(&optval), sizeof(optval)) == -1)
	{
		closesocket(sock);
		OSDEBUG("set reuseaddr error!!");
		return INVALID_SOCKET_ID;
	}

	if(linux_type == SOCK_DGRAM)
	{
		optval = 1;
		if(setsockopt(sock, IPPROTO_IP, IP_PKTINFO, (const char *)(&optval), sizeof(optval)) == -1)
		{
			OSLOG("setup IPPROTO_IP failed!");
		}
	}

	optval = 1024 * 128;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char *)(&optval), sizeof(optval)) == -1)
	{
		closesocket(sock);
		OSDEBUG("set rcv buf error!!");
		return INVALID_SOCKET_ID;
	}

	optval = 1024 * 64;
	if(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char *)(&optval), sizeof(optval)) == -1)
	{
		closesocket(sock);
		OSDEBUG("set snd buf error!!");
		return INVALID_SOCKET_ID;
	}
	
	optval = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVLOWAT, (const char *)(&optval), sizeof(optval)) == -1)
	{
		OSDEBUG("set rcv low error!!");
	}

	optval = 0;
	optval_len = sizeof(optval);
	getsockopt(sock, SOL_SOCKET, SO_SNDLOWAT, (char *)(&optval), &optval_len);

	/* Disable the Nagle (TCP No Delay) algorithm */
	if(linux_type == SOCK_STREAM)
	{
		optval = 0;
		if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)(&optval), sizeof(optval)) == -1)
		{
			closesocket(sock);
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
			closesocket(sock);
			OSDEBUG("set socket error!");	
			return INVALID_SOCKET_ID;  
		}

	}

	return sock;
}

static void *
_os_cygwin_socket_event_thread(void *arg)
{
    WSAEVENT eventArray[WSA_MAXIMUM_WAIT_EVENTS];
    SOCKET sockArray[WSA_MAXIMUM_WAIT_EVENTS];
    int nEvent = 0;
	int nIndex;
	SOCKET sock;
	WSANETWORKEVENTS event;

	while(dave_os_thread_canceled(_cygwin_socket_event_thread) == dave_false)
	{
		/*
		 * WSAWaitForMultipleEvents 机制有一个问题，如果在调用WSAWaitForMultipleEvents
		 * 之前，有套接字没有就绪，而是调用之后再就绪的，这个时候只能靠其自身的超时
		 * 机制，退出WSAWaitForMultipleEvents，在下次线程循环里面才能检测事件。
		 * 好在这个定时很短。
		 * 目前看，最好的方式是改成每个套接字一个线程来做事件监听。
		 * 这样也能解决WSAWaitForMultipleEvents一次只能监听WSA_MAXIMUM_WAIT_EVENTS=64
		 * 个套接字的问题。
		 */
		nEvent = _os_cygwin_load_event_array(eventArray, sockArray);
		if(nEvent == 0)
		{
			dave_os_thread_sleep(_cygwin_socket_event_thread);
			continue;
		}

		nIndex = WSAWaitForMultipleEvents(nEvent, eventArray, false, WSA_WAIT_TIMEOUT, false);
		if( nIndex == WSA_WAIT_IO_COMPLETION || nIndex == WSA_WAIT_TIMEOUT )
		{
			OSDEBUG("nIndex:%lx/%lx error:%d", nIndex, WSA_WAIT_TIMEOUT, WSAGetLastError());
			continue;
		}

		nIndex = nIndex - WSA_WAIT_EVENT_0;
		if((nIndex < 0) || (nIndex >= WSA_MAXIMUM_WAIT_EVENTS))
		{
			continue;
		}

		sock = sockArray[nIndex - WSA_WAIT_EVENT_0];
		WSAEnumNetworkEvents(sock, eventArray[nIndex], &event);

		if(event.lNetworkEvents & FD_ACCEPT)
		{
			_os_cygwin_notify_event(SOC_EVENT_REV, sock);
		}
		else if(event.lNetworkEvents & FD_READ)
		{
			_os_cygwin_notify_event(SOC_EVENT_REV, sock);
		}
		else if(event.lNetworkEvents & FD_CLOSE)
		{
			_os_cygwin_notify_event(SOC_EVENT_CLOSE, sock);
		}
		else if(event.lNetworkEvents & FD_WRITE)
		{
			OSDEBUG("socket:%d write", sock);
		}
		else
		{
			OSDEBUG("socket:%d lost event:%x", sock, event.lNetworkEvents);
		}
	}

	return NULL;
}

// =====================================================================

dave_bool
dave_os_socket_init(dave_socket_event_fun event_call_back)
{
	WORD wVersionRequested = WINSOCK_VERSION;
	WSADATA wsaData;
	int error;

	_cygwin_event_notify_fun = event_call_back;

	error = WSAStartup(wVersionRequested, &wsaData);
	if(error != 0)
	{
		WSACleanup();
		return dave_false;
	}

	pthread_mutex_init(&m_event_mutex, NULL);

	_os_cygwin_event_reset();

	_cygwin_socket_event_thread = dave_os_create_thread("event", _os_cygwin_socket_event_thread, NULL);
	if(_cygwin_socket_event_thread == NULL)
	{
		OSABNOR("event thread failed!");
	}

	return dave_true;
}

dave_bool
dave_os_socket_exit(void)
{
	WSACleanup();

	if(_cygwin_socket_event_thread != NULL)
	{
		dave_os_release_thread(_cygwin_socket_event_thread);
	}

	pthread_mutex_destroy(&m_event_mutex);

	return dave_true;
}

s32
dave_os_socket(SOCDOMAIN domain, SOCTYPE type, NetAddrType addr_type, s8 *netcard_name)
{
	int cygwin_domain = _os_cygwin_domain(domain);
	int cygwin_type = _os_cygwin_net_type(type);
	int sock;

	sock = socket(cygwin_domain, cygwin_type, 0);
	if(sock == INVALID_SOCKET)
	{
		OSLOG("domain:%s type:%s netcard_name:%s creat socket failed!",
			t_auto_SOCDOMAIN_str(domain), t_auto_SOCTYPE_str(type),
			netcard_name);
		return -1;
	}

	OSDEBUG("socket:%d", sock);

	return sock;
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
		if(_os_cygwin_socket_bind_fix_port(socket, pNetInfo->src_port) == dave_false)
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
		_os_cygwin_keepalive_setup_socket(socket, pNetInfo->keepalive_second);
	}

	ret = connect(socket, (struct sockaddr *)&addr, sizeof(addr));

	if(ret < 0)
	{
		if(WSAGetLastError() != 0)
		{
			OSLOG("ret:%d socket:%d errno:%d", ret, socket, WSAGetLastError());
			return SOC_CNT_FAIL;
		}
	}

	getsockname(socket, (struct sockaddr *)&guest, &guest_len);
	pNetInfo->src_port = ntohs(guest.sin_port);

	_os_cygwin_add_event(socket);

	OSDEBUG("socket:%d %s ret:%d", socket, ipv4str(pNetInfo->addr.ip.ip_addr, pNetInfo->port), ret);

	return SOC_CNT_OK;
}

dave_bool
dave_os_bind(s32 socket, SocNetInfo *pNetInfo)
{
	dave_bool bind_v4_ret, bind_v6_ret;

	bind_v4_ret = _os_cygwin_socket_bind_ip_v4(socket, pNetInfo);
	bind_v6_ret = _os_cygwin_socket_bind_ip_v6(socket, pNetInfo);

	if((bind_v4_ret == dave_true) || (bind_v6_ret == dave_true))
	{
		OSDEBUG("socket:%d bind success!", socket);
		return dave_true;
	}
	else
	{
		OSDEBUG("errno:%d", WSAGetLastError());
		return dave_false;
	}
}

dave_bool
dave_os_listen(s32 socket, sw_uint32 backlog)
{
	if(listen(socket, backlog) >= 0)
	{
		_os_cygwin_add_event(socket);

		OSDEBUG("socket:%d", socket);

		return dave_true;
	}
	else
	{
		OSABNOR("socket listen failed: errno(%d)", WSAGetLastError());
		return dave_false;
	}
}

s32
dave_os_accept(s32 socket, SocNetInfo *pNetInfo)
{
	int cygwin_type = _os_cygwin_net_type(pNetInfo->type);
	SOCKET  child_socket;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);

	child_socket = accept(socket, (struct sockaddr *)&addr, &len);
	if(child_socket == INVALID_SOCKET)
	{
		OSDEBUG("socket:%d accept fail! child_socket:%d errno(%d)",
			socket, child_socket, WSAGetLastError());
		return -1;
	}

	strip((s8 *)inet_ntoa(addr.sin_addr), dave_strlen((s8 *)inet_ntoa(addr.sin_addr)), pNetInfo->addr.ip.ip_addr, 16);
	pNetInfo->port = ntohs(addr.sin_port);

	OSDEBUG("accept socket=%d/%d %s",
		child_socket, socket,
		ipv4str(pNetInfo->addr.ip.ip_addr, pNetInfo->port));

	if(pNetInfo->enable_keepalive_flag == KeepAlive_enable)
	{
		OSDEBUG("socket:%d enable keepalive!", child_socket);
		_os_cygwin_keepalive_setup_socket(child_socket, pNetInfo->keepalive_second);
	}

	return _os_cygwin_normal_setup_socket(cygwin_type, pNetInfo->addr_type, child_socket);
}

void
dave_os_epoll(s32 socket)
{
	_os_cygwin_add_event(socket);
}

dave_bool
dave_os_recv(s32 socket, SocNetInfo *pNetInfo, u8 *data, ub *data_len)
{
	int recv_len;

	recv_len = recv(socket, (char *)data, *data_len, 0);

	if(recv_len == 0)
	{
		*data_len = 0;
		if(pNetInfo->type == TYPE_SOCK_DGRAM)
		{
			return dave_true;
		}
		OSDEBUG("socket:%d errno:%d", socket, WSAGetLastError());
		return dave_false;
	}
	else if(recv_len < 0)
	{
		OSDEBUG("socket:%d recv_len:%d error:%d", socket, recv_len, WSAGetLastError());
		*data_len = 0;
		if(pNetInfo->type == TYPE_SOCK_DGRAM)
		{
			return dave_true;
		}
		else
		{
			if(WSAGetLastError() == WSAEWOULDBLOCK)
				return dave_true;
			else
				return dave_false;
		}
	}
	else
	{
		OSDEBUG("socket:%d recv_len:%d", socket, recv_len);
		*data_len = (ub)recv_len;
		return dave_true;
	}
}

sb
dave_os_send(s32 socket, SocNetInfo *pNetInfo, u8 *data, ub data_len, dave_bool urg)
{
	sb snd_len;
	struct sockaddr_in addr;
	s8 ip_str[20];

	if(socket < 0)
	{
		OSABNOR("invalid socket:%d", socket);
		return -1;
	}

	if(pNetInfo->type == TYPE_SOCK_STREAM)
	{
		snd_len = send(socket, (char *)data, data_len, 0);

		OSDEBUG("socket:%d data_len:%d snd_len:%d", socket, data_len, snd_len);

		if(snd_len >= 0)
		{
			return snd_len;
		}
		else
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				return 0;
			}
			else
			{					
				OSDEBUG("socket:%d snd_len:%d data_len:%d ret:%d errno:%d",
					socket, snd_len, data_len, snd_len, WSAGetLastError());
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

		snd_len = sendto(socket, (char *)data, data_len, 0, (struct sockaddr *)(&addr), sizeof(struct sockaddr_in));
		if (snd_len >= 0)
		{
			return snd_len;
		}
		else
		{
			if(WSAGetLastError() == 0)
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

	return 0;
}

dave_bool
dave_os_close(s32 socket, dave_bool clean_wait)
{
	OSDEBUG("socket:%d", socket);

	if(socket >= 0)
	{
		_os_cygwin_del_event(socket);

		closesocket(socket);
	}

	return dave_true;
}

dave_bool
dave_os_gethostbyname(s8 *ip_ptr, ub ip_len, char *domain)
{
	struct hostent *h;

	dave_memset(ip_ptr, 0x00, ip_len);

	if(t_is_ipv4(domain) == dave_true)
	{
		dave_strcpy(ip_ptr, domain, ip_len);
		return dave_true;
	}

	/*
	 * 还需解决的问题：
	 * 如果domain是一个非法的值，会使得gethostbyname
	 * 等待很长时间。
	 */
	h = gethostbyname(domain);
	if(h ==NULL)
	{
		return dave_false;
	}

	dave_strcpy(ip_ptr, inet_ntoa(*((struct in_addr *)h->h_addr)), ip_len);

	return dave_true;
}

#endif

