/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "socket_tools.h"
#include "socket_external.h"
#include "socket_parameters.h"
#include "socket_snd_list.h"
#include "socket_log.h"

#define RECV_COUNTER_MAX (32)
#define SEND_COUNTER_MAX (16384)
#define SEND_MBUF_MAX (256)
#define GAVE_EVENT_MAX (32)
#define WAIT_EVENT_MAX (16)

static ThreadId _socket_thread = INVALID_THREAD_ID;

static void
_socket_external_raw_event(SOCEVENT event, s32 socket, s32 os_socket)
{
	SocketRawEvent *pEvent;

	pEvent = thread_msg(pEvent);
	pEvent->socket = socket;
	pEvent->os_socket = os_socket;
	pEvent->event = event;
	pEvent->data = NULL;

	snd_from_msg(_socket_thread, _socket_thread, SOCKET_RAW_EVENT, sizeof(SocketRawEvent), pEvent);
}

static void
_socket_external_core_event(SOCEVENT event, SocketCore *pCore)
{
	if(event == SOC_EVENT_CLOSE)
	{
		if(pCore->wait_close == dave_true)
		{
			return;
		}

		pCore->wait_close = dave_true;
	}

	_socket_external_raw_event(event, pCore->socket_external_index, pCore->os_socket);
}

static inline void
_socket_external_os_event(SOCEVENT event, s32 os_socket, dave_bool level_trigger)
{
	_socket_external_raw_event(event, INVALID_SOCKET_ID, os_socket);
}

static inline void
_socket_external_send_counter(SocketCore *pCore, ub send_length)
{
	t_lock_spin(&(pCore->opt_pv));
	pCore->data_send_length += send_length;
	t_unlock_spin(&(pCore->opt_pv));
}

static inline void
_socket_external_recv_counter(SocketCore *pCore, ub recv_length)
{
	t_lock_spin(&(pCore->opt_pv));
	pCore->data_recv_length += recv_length;
	t_unlock_spin(&(pCore->opt_pv));
}

static inline void
_socket_external_plugout(ThreadId dst, SOCKETTYPE type, s32 socket_external_index, SocNetInfo *pNetInfo, void *user_ptr)
{
	SocketPlugOut *pPlugout;

	if((pNetInfo == NULL)
		|| (type == SOCKET_TYPE_SERVER_FATHER)
		|| (type == SOCKET_TYPE_SERVER_CHILD)
		|| (type == SOCKET_TYPE_CLIENT)
		|| (type == SOCKET_TYPE_CLIENT_WAIT)
		|| (pNetInfo->type == TYPE_SOCK_DGRAM))
	{
		pPlugout = thread_msg(pPlugout);

		pPlugout->socket = socket_external_index;
		pPlugout->reason = SOCKETINFO_DISCONNECT_OK;
		T_CopyNetInfo(&(pPlugout->NetInfo), pNetInfo);
		pPlugout->thread_id = _socket_thread;
		pPlugout->ptr = user_ptr;

		write_msg(dst, SOCKET_PLUGOUT, pPlugout);
	}
}

static inline void
_socket_external_plugin(SocketCore *pCore, ThreadId dst, s32 father_socket_external_index, s32 child_socket_external_index, SocNetInfo *pNetInfo)
{
	ub safe_counter;
	SocketPlugIn *pPlugin = thread_msg(pPlugin);

	safe_counter = 0;

	while((pCore->bind_or_connect_rsp_flag == dave_false) && ((++ safe_counter) < 128))
	{
		dave_os_nsleep(1);
	}

	pPlugin->father_socket = father_socket_external_index;
	pPlugin->child_socket = child_socket_external_index;
	T_CopyNetInfo(&(pPlugin->NetInfo), pNetInfo);
	pPlugin->thread_id = _socket_thread;
	pPlugin->ptr = pCore->user_ptr;

	write_msg(dst, SOCKET_PLUGIN, pPlugin);
}

static inline void
_socket_external_recv_buf_adjustment(SocketCore *pCore, ub recv_real_length)
{
	dave_bool has_adjustment = dave_false;

	if(pCore->NetInfo.type == TYPE_SOCK_STREAM)
	{
		if(recv_real_length >= pCore->tcp_recv_buf_length)
		{
			if((pCore->tcp_recv_buf_length + SOCKET_TCP_RECV_ADD_GRADIENT) <= SOCKET_TCP_RECV_MAX_BUF)
			{
				pCore->tcp_recv_buf_length += SOCKET_TCP_RECV_ADD_GRADIENT;
				has_adjustment = dave_true;
			}
		}

		if(has_adjustment == dave_true)
		{
			SOCKETDEBUG("socket:%d/%d port:%d owner:%s adjustment:%d/%d",
				pCore->socket_external_index, pCore->socket_internal_index,
				pCore->NetInfo.port, thread_name(pCore->owner),
				recv_real_length, pCore->tcp_recv_buf_length);
		}
	}
}

static inline void
_socket_external_build_write_ipinfo(SocNetInfo *pNetInfo, SocketCore *pCore, IPBaseInfo *pIPInfo)
{
	T_CopyNetInfo(pNetInfo, &(pCore->NetInfo));
	if((pIPInfo != NULL) && (pCore->NetInfo.type == TYPE_SOCK_DGRAM) && (pIPInfo->protocol == IPProtocol_UDP))
	{
		if(pIPInfo->dst_port != 0)
		{
			dave_memcpy(pNetInfo->addr.ip.ip_addr, pIPInfo->dst_ip, 16);
			pNetInfo->port = pIPInfo->dst_port;
		}
	}
}

static inline void
_socket_external_build_read_ipinfo(SocketCore *pCore, SocketRead *pRead, SocNetInfo *recv_info)
{
	if(pCore->NetInfo.type == TYPE_SOCK_DGRAM)
	{
		if(pCore->NetInfo.domain == DM_SOC_PF_INET6)
		{
			BuildIPInfo(&(pRead->IPInfo), IPVER_IPV6,
				IPProtocol_UDP,
				recv_info->src_ip.ip_addr, recv_info->src_port,
				pCore->NetInfo.addr.ip.ip_addr, pCore->NetInfo.port);
		}
		else
		{
			BuildIPInfo(&(pRead->IPInfo), IPVER_IPV4,
				IPProtocol_UDP,
				recv_info->src_ip.ip_addr, recv_info->src_port,
				pCore->NetInfo.addr.ip.ip_addr, pCore->NetInfo.port);
		}
	}
	else
	{
		BuildIPInfo(&(pRead->IPInfo), IPVER_IPV4,
			IPProtocol_TCP,
			pCore->NetInfo.addr.ip.ip_addr, pCore->NetInfo.port,
			pCore->NetInfo.src_ip.ip_addr, pCore->NetInfo.src_port);
	}
}

static inline dave_bool
_socket_external_recv_notify(SocketCore *pCore, SocNetInfo *pNetInfo, MBUF *data)
{
	SocketRawEvent *pEvent = thread_msg(pEvent);

	pEvent->socket = pCore->socket_external_index;
	pEvent->os_socket = pCore->os_socket;
	pEvent->event = SOC_EVENT_REV;
	if(data != NULL)
	{
		T_CopyNetInfo(&(pEvent->NetInfo), pNetInfo);
		pEvent->data = data;
	}
	else
	{
		T_CopyNetInfo(&(pEvent->NetInfo), &(pCore->NetInfo));
		pEvent->data = NULL;
	}
	pEvent->ptr = pCore;

	return write_nmsg(pCore->owner, SOCKET_RAW_EVENT, pEvent, 128);
}

static inline dave_bool
_socket_external_recv_data(SocketCore *pCore, SocketRawEvent *pEvent)
{
	SocketRead *pRead = thread_msg(pRead);
	SocNetInfo recv_info;

	T_CopyNetInfo(&recv_info, &(pCore->NetInfo));

	pRead->socket = pCore->socket_external_index;
	_socket_external_build_read_ipinfo(pCore, pRead, &recv_info);
	pRead->data_len = pEvent->data->len;
	pRead->data = pEvent->data;
	pRead->ptr = pCore->user_ptr;

	write_msg(pCore->owner, SOCKET_READ, pRead);

	return dave_true;
}

static inline dave_bool
_socket_external_recv_package(SocketCore *pCore)
{
	MBUF *data;
	SocNetInfo recv_info;
	SocketRead *pRead;
	dave_bool ret = dave_false;

	if(pCore->NetInfo.type == TYPE_SOCK_STREAM)
	{
		if(pCore->tcp_recv_buf_length > SOCKET_TCP_RECV_MAX_BUF)
		{
			SOCKETLOG("invalid tcp_recv_buf_length:%d on socket:%d/%d thread:%s port:%d/%d",
				pCore->tcp_recv_buf_length,
				pCore->socket_external_index, pCore->socket_internal_index,
				thread_name(pCore->owner),
				pCore->NetInfo.port, pCore->NetInfo.src_port);
			pCore->tcp_recv_buf_length = SOCKET_TCP_RECV_MAX_BUF;
		}

		data = dave_mmalloc(pCore->tcp_recv_buf_length);
	}
	else
	{
		data = dave_mmalloc(SOCKET_UDP_RECV_MAX_BUF);
	}

	T_CopyNetInfo(&recv_info, &(pCore->NetInfo));

	if(dave_os_recv(pCore->os_socket, &recv_info, (u8 *)(data->payload), (ub *)(&(data->len))) == dave_false)
	{
		_socket_external_core_event(SOC_EVENT_CLOSE, pCore);
	}
	else
	{
		if(data->len > 0)
		{
			_socket_external_recv_counter(pCore, data->len);

			if(data->len < data->tot_len)
			{
				((s8 *)(data->payload))[data->len] = '\0';
			}

			_socket_external_recv_buf_adjustment(pCore, data->len);

			pRead = thread_msg(pRead);

			pRead->socket = pCore->socket_external_index;
			_socket_external_build_read_ipinfo(pCore, pRead, &recv_info);
			pRead->data_len = data->tot_len = data->len;
			pRead->data = data;
			pRead->ptr = pCore->user_ptr;

			SOCKETDEBUG("%s socket:%d/%d/%d %s->%s recv len:%d",
				thread_name(pCore->owner),
				pCore->socket_external_index, pCore->socket_internal_index, pCore->os_socket,
				ipv4str(pRead->IPInfo.src_ip, pRead->IPInfo.src_port),
				ipv4str2(pRead->IPInfo.dst_ip, pRead->IPInfo.dst_port),
				pRead->data_len);

			write_msg(pCore->owner, SOCKET_READ, pRead);

			ret = dave_true;
		}
	}

	if(ret == dave_false)
	{
		dave_mfree(data);
	}

	return ret;
}

static inline void
_socket_external_maybe_has_data(SocketCore *pCore)
{
	_socket_external_core_event(SOC_EVENT_REV, pCore);
}

static inline void
_socket_external_recv(SocketCore *pCore, SocketRawEvent *pEvent)
{
	ub safe_counter;
	dave_bool ret;

	if(pCore->use_flag == dave_false)
	{
		SOCKETLTRACE(60,1,"socket:%d/%d is close!", pCore->socket_external_index, pCore->socket_internal_index);
		return;
	}

	safe_counter = 0;

	if(pCore->NetInfo.domain == DM_SOC_PF_RAW_INET)
	{
		ret = _socket_external_recv_notify(pCore, &(pEvent->NetInfo), pEvent->data);
	}
	else if(pEvent->data != NULL)
	{
		ret = _socket_external_recv_data(pCore, pEvent);
	}
	else
	{
		ret = dave_true;

		while(safe_counter <= RECV_COUNTER_MAX)
		{
			ret = _socket_external_recv_package(pCore);
			if(ret == dave_false)
			{
				break;
			}

			safe_counter ++;
		}
	}

	if((ret == dave_true) && (safe_counter > RECV_COUNTER_MAX))
	{
		SOCKETLOG("maybe the %s's socket:%d/%d not received completed!",
			thread_name(pCore->owner),
			pCore->socket_external_index, pCore->socket_internal_index);
	}

	if(safe_counter >= RECV_COUNTER_MAX)
	{
		_socket_external_maybe_has_data(pCore);
	}
}

static inline dave_bool
_socket_external_send(SocketCore *pCore, IPBaseInfo *pIPInfo, u8 *data, sb data_len, SOCKETINFO snd_flag)
{
	dave_bool urg = (snd_flag == SOCKETINFO_SND_URG) ? dave_true : dave_false;
	sb data_index, send_counter;
	ub snd_start_time;
	SocNetInfo NetInfo;
	sb send_len, ret_error;

	if((data == NULL) || (data_len < 0))
	{
		SOCKETABNOR("invalid data:%x or data_len:%d", data, data_len);
		return dave_false;
	}

	data_index = send_counter = 0;

	snd_start_time = dave_os_time_us();

	_socket_external_build_write_ipinfo(&NetInfo, pCore, pIPInfo);

	while(((++  send_counter) < SEND_COUNTER_MAX) && (data_index < data_len))
	{
		send_len = dave_os_send(
			pCore->os_socket, &NetInfo,
			&data[data_index], data_len-data_index,
			urg);

		if(send_len < 0)
		{
			SOCKETTRACE("%s port:%d/%d ip:%d.%d.%d.%d/%d.%d.%d.%d socket:%d/%d/%d data:%d send:%d %s",
				thread_name(pCore->owner),
				pCore->NetInfo.port, pCore->NetInfo.src_port,
				pCore->NetInfo.addr.ip.ip_addr[0], pCore->NetInfo.addr.ip.ip_addr[1], pCore->NetInfo.addr.ip.ip_addr[2], pCore->NetInfo.addr.ip.ip_addr[3],
				pCore->NetInfo.src_ip.ip_addr[0], pCore->NetInfo.src_ip.ip_addr[1], pCore->NetInfo.src_ip.ip_addr[2], pCore->NetInfo.src_ip.ip_addr[3],
				pCore->socket_external_index, pCore->socket_internal_index, pCore->os_socket,
				data_len, send_len,
				dave_os_errno(NULL));

			return dave_false;
		}
		else if(send_len == 0)
		{
			dave_os_errno(&ret_error);

			// Resource temporarily unavailable
			if(ret_error == 11)
			{
				dave_os_usleep(send_counter);
			}
			else
			{
				dave_os_usleep(1);
			}
		}
		else
		{
			data_index += send_len;

			send_counter = 0;
		}
	}

	if((send_counter >= SEND_COUNTER_MAX) || (data_index < data_len))
	{
		SOCKETABNOR("%s socket:%d/%d counter:%d index:%d len:%d %s time:%ldus",
			thread_name(pCore->owner),
			pCore->socket_external_index, pCore->socket_internal_index,
			send_counter, data_index, data_len,
			dave_os_errno(NULL),
			dave_os_time_us() - snd_start_time);

		return dave_false;
	}
	else
	{
		_socket_external_send_counter(pCore, data_index);

		return dave_true;
	}
}

static inline dave_bool
_socket_external_accept_child(SocketCore *pFatherCore)
{
	SocNetInfo accept_info;
	s32 os_socket;
	SocketCore *pChildCore;

	T_CopyNetInfo(&accept_info, &(pFatherCore->NetInfo));

	os_socket = dave_os_accept(pFatherCore->os_socket, &accept_info);
	if(os_socket < 0)
	{
		SOCKETDEBUG("%s socket:%d/%d/%d accept failed!",
			thread_name(pFatherCore->owner),
			pFatherCore->socket_external_index, pFatherCore->socket_internal_index, pFatherCore->os_socket);

		return dave_false;
	}

	pChildCore = socket_core_malloc(pFatherCore->owner, SOCKET_TYPE_SERVER_CHILD, &accept_info, pFatherCore->user_ptr, os_socket);
	if(pChildCore == NULL)
	{
		SOCKETABNOR("socket:%d/%d malloc pChildCore failed!",
			pFatherCore->socket_external_index, pFatherCore->socket_internal_index);

		dave_os_close(os_socket, dave_false);

		return dave_false;
	}

	pChildCore->NetInfo.src_ip.ver = pFatherCore->NetInfo.addr.ip.ver;
	dave_memcpy(pChildCore->NetInfo.src_ip.ip_addr, pFatherCore->NetInfo.addr.ip.ip_addr, sizeof(pChildCore->NetInfo.src_ip.ip_addr));
	pChildCore->NetInfo.src_port = pFatherCore->NetInfo.port;

	SOCKETDEBUG("%s father:%x/%d/%d/%d client:%x/%d/%d/%d src_port:%d success!",
		thread_name(pChildCore->owner),
		pFatherCore, pFatherCore->socket_external_index, pFatherCore->socket_internal_index, pFatherCore->os_socket,
		pChildCore, pChildCore->socket_external_index, pChildCore->socket_internal_index, pChildCore->os_socket,
		pChildCore->NetInfo.src_port);

	_socket_external_plugin(pChildCore, pChildCore->owner, pFatherCore->socket_external_index, pChildCore->socket_external_index, &(pChildCore->NetInfo));

	dave_os_epoll(os_socket);

	return dave_true;
}

static inline void
_socket_external_accept(SocketCore *pCore)
{
	ub safe_counter;
	dave_bool ret;

	if(pCore->use_flag == dave_false)
	{
		SOCKETTRACE("socket:%d/%d is close!",
			pCore->socket_external_index, pCore->socket_internal_index);
		return;
	}

	safe_counter = 0;

	ret = dave_true;

	while(((++ safe_counter) < SOCKET_MAX) && (ret == dave_true))
	{
		ret = _socket_external_accept_child(pCore);
	}

	if(safe_counter >= SOCKET_MAX)
	{
		SOCKETABNOR("socket:%d/%d port:%s owner:%s maybe not accept!",
			pCore->socket_external_index, pCore->socket_internal_index,
			pCore->NetInfo.port, thread_name(pCore->owner));
	}
}

static inline void
_socket_external_connect_event(SocketCore *pCore)
{
	if(pCore->use_flag == dave_false)
	{
		SOCKETTRACE("socket:%d/%d is close!",
			pCore->socket_external_index, pCore->socket_internal_index);
		return;
	}

	if(pCore->type == SOCKET_TYPE_CLIENT_WAIT)
	{
		_socket_external_plugin(pCore, pCore->owner, INVALID_SOCKET_ID, pCore->socket_external_index, &(pCore->NetInfo));

		pCore->type = SOCKET_TYPE_CLIENT;
	}
	else
	{
		SOCKETABNOR("socket:%d/%d/%d type:%d has invalid event!",
			pCore->socket_external_index, pCore->socket_internal_index, pCore->os_socket,
			pCore->type);
	}
}

static inline void
_socket_external_connect_failed_event(SocketCore *pCore)
{
	s32 os_socket;

	if(pCore->use_flag == dave_false)
	{
		SOCKETTRACE("socket:%d/%d is close!",
			pCore->socket_external_index, pCore->socket_internal_index);
		return;
	}
	if(pCore->os_socket < 0)
	{
		SOCKETABNOR("%s has invalid os_socket:%d socket_index:%d/%d!",
			thread_name(pCore->owner), pCore->os_socket,
			pCore->socket_external_index, pCore->socket_internal_index);
		return;
	}
	if(pCore->type != SOCKET_TYPE_CLIENT_WAIT)
	{
		SOCKETLOG("socket:%d/%d/%d type:%d has invalid event!",
			pCore->socket_external_index, pCore->socket_internal_index, pCore->os_socket,
			pCore->type);
	}

	os_socket = pCore->os_socket;

	_socket_external_plugout(pCore->owner, SOCKET_TYPE_CLIENT, pCore->socket_external_index, &(pCore->NetInfo), pCore->user_ptr);		

	socket_core_free(pCore);

	dave_os_close(os_socket, dave_true);
}

static inline void
_socket_external_close_event(SocketCore *pCore)
{
	s32 os_socket;
	dave_bool clean_wait;

	if(pCore->use_flag == dave_false)
	{
		SOCKETTRACE("socket:%d/%d is close!",
			pCore->socket_external_index, pCore->socket_internal_index);
		return;
	}

	if((pCore->type == SOCKET_TYPE_CLIENT) || (pCore->type == SOCKET_TYPE_CLIENT_WAIT))
	{
		clean_wait = dave_true;
	}
	else
	{
		clean_wait = dave_false;
	}

	os_socket = pCore->os_socket;

	_socket_external_plugout(pCore->owner, pCore->type, pCore->socket_external_index, &(pCore->NetInfo), pCore->user_ptr);

	socket_core_free(pCore);

	dave_os_close(os_socket, clean_wait);
}

static inline SocketCore *
_socket_external_creat_service(ThreadId src, SocNetInfo *pNetInfo, void *user_ptr)
{
	s32 os_socket;
	ErrCode ret = ERRCODE_invalid_option;
	SocketCore *pCore = NULL;

	os_socket = dave_os_socket(pNetInfo->domain, pNetInfo->type, pNetInfo->addr_type, pNetInfo->netcard_bind_flag==NetCardBind_enable?pNetInfo->netcard_name:NULL);
	if(os_socket >= 0)
	{
		pCore = socket_core_malloc(src, SOCKET_TYPE_SERVER_FATHER, pNetInfo, user_ptr, os_socket);

		if(dave_os_bind(os_socket, pNetInfo) == dave_true)
		{
			if(pNetInfo->type == TYPE_SOCK_STREAM)
			{
				if(dave_os_listen(os_socket, 8192) == dave_true)
				{
					ret = ERRCODE_OK;
				}
			}
			else
			{
				ret = ERRCODE_OK;
			}
		}
	}

	if((ret != ERRCODE_OK) || (pCore == NULL))
	{
		SOCKETTRACE("%s/%s",
			ipv4str(pNetInfo->addr.ip.ip_addr, pNetInfo->port),
			dave_os_errno(NULL));

		socket_core_free(pCore);

		dave_os_close(os_socket, dave_false);

		return NULL;
	}

	return pCore;
}

static inline SocketCore *
_socket_external_connect_service(ThreadId src, SocNetInfo *pNetInfo, void *user_ptr, SOCKETINFO *ConnectInfo)
{
	s32 os_socket;
	SOCCNTTYPE cnt_type;
	SocketCore *pCore = NULL;

	*ConnectInfo = SOCKETINFO_CONNECT_FAIL;

	os_socket = dave_os_socket(pNetInfo->domain, pNetInfo->type, pNetInfo->addr_type, pNetInfo->netcard_bind_flag==NetCardBind_enable?pNetInfo->netcard_name:NULL);
	if(os_socket < 0)
	{
		SOCKETABNOR("%s creat socket failed!", thread_name(src));
		return NULL;
	}

	if(pNetInfo->type == TYPE_SOCK_STREAM)
	{
		pCore = socket_core_malloc(src, SOCKET_TYPE_CLIENT_WAIT, pNetInfo, user_ptr, os_socket);

		cnt_type = dave_os_connect(os_socket, pNetInfo);

		SOCKETDEBUG("os_socket:%d cnt_type:%d", os_socket, cnt_type);

		switch(cnt_type)
		{
			case SOC_CNT_OK:
					*ConnectInfo = SOCKETINFO_CONNECT_OK;
					SAFEZONEv5W(pCore->opt_pv, { pCore->type = SOCKET_TYPE_CLIENT; } );
				break;
			case SOC_CNT_FAIL:
					*ConnectInfo = SOCKETINFO_CONNECT_FAIL;
				break;
			case SOC_CNT_WAIT:
					*ConnectInfo = SOCKETINFO_CONNECT_WAIT;
				break;
			default:
					*ConnectInfo = SOCKETINFO_CONNECT_FAIL;
				break;
		}
	}
	else
	{
		*ConnectInfo = SOCKETINFO_CONNECT_OK;

		pCore = socket_core_malloc(src, SOCKET_TYPE_CLIENT, pNetInfo, user_ptr, os_socket);
	}

	if((*ConnectInfo != SOCKETINFO_CONNECT_OK) && (*ConnectInfo != SOCKETINFO_CONNECT_WAIT))
	{
		socket_core_free(pCore);

		dave_os_close(os_socket, dave_true);

		return NULL;
	}

	return pCore;
}

static inline dave_bool
_socket_external_output(SocketCore *pCore, IPBaseInfo *pIPInfo, MBUF *data, SOCKETINFO snd_flag)
{
	u8 *data_buf;
	ub data_length;
	ub safe_counter;
	dave_bool ret;

	ret = dave_false;

	if(pCore->NetInfo.type == TYPE_SOCK_DGRAM)
	{
		/*
		 * 对于UDP包，如果不是一次性提交给系统，那么很有可能会发生一个UDP包
		 * 被系统分多次发送，而这多次的到达时间可能不会按顺序要求。
		 */
		data_length = data->tot_len;

		data_buf = dave_malloc(data_length);

		data_length = t_a2b_mbuf_to_buf(data, data_buf, data_length);
		
		ret = _socket_external_send(pCore, pIPInfo, (u8 *)(data_buf), (sb)(data_length), snd_flag);

		dave_free(data_buf);
	}
	else
	{
		safe_counter = 0;

		ret = dave_true;

		while(((safe_counter ++) < SEND_MBUF_MAX) && (data != NULL) && (ret == dave_true))
		{
			ret = _socket_external_send(pCore, pIPInfo, (u8 *)(data->payload), (sb)(data->len), snd_flag);

			data = data->next;
		}

		if(safe_counter >= SEND_MBUF_MAX)
		{
			SOCKETABNOR("invalid safe_counter:%d", safe_counter);
		}
	}

	return ret;
}

static inline void
_socket_external_socket_file_max(void)
{
	sb file_max, new_file_max;
	dave_bool ret;

	file_max = dave_os_get_system_file_max();
	if(file_max < 0)
	{
		SOCKETABNOR("invalid file_max:%d", file_max);
		return;
	}

	if(file_max < SOCKET_MAX)
	{
		new_file_max = SOCKET_MAX;

		ret = dave_os_set_system_file_max(new_file_max);

		if(ret == dave_true)
		{
			SOCKETTRACE("set socket file from %d to %d success!",
				file_max, new_file_max);
		}
		else
		{
			SOCKETLOG("Maybe not a ROOT user, set socket file from %d to %d failed!",
				file_max, new_file_max)
		}
	}
}

static inline dave_bool
_socker_external_list_output(SocketCore *pCore, IPBaseInfo *pIPInfo, MBUF *data, SOCKETINFO snd_flag)
{
	dave_bool snd_token;
	SokcetSndList *pSndData;
	dave_bool ret;

	snd_token = socket_snd_list_catch_snd_token(pCore, pIPInfo, data, snd_flag);

	if(snd_token == dave_true)
	{
		ret = dave_true;

		while(ret == dave_true)
		{
			pSndData = socket_snd_list_catch_data(pCore);
			if(pSndData != NULL)
			{
				ret = _socket_external_output(pCore, &(pSndData->IPInfo), pSndData->data, pSndData->snd_flag);
				if(ret == dave_false)
				{
					SOCKETTRACE("%s socket:%d/%d failed! %s",
						thread_name(pCore->owner),
						pCore->socket_external_index, pCore->socket_internal_index,
						dave_os_errno(NULL));

					socket_snd_list_clean(pCore);

					_socket_external_core_event(SOC_EVENT_CLOSE, pCore);
				}

				socket_snd_list_release_data(pSndData);
			}
			else
			{
				ret = dave_false;
			}
		}

		socket_snd_list_release_snd_token(pCore);
	}

	return snd_token;
}

static inline dave_bool
_socket_external_send_event(SocketCore *pCore, IPBaseInfo *pIPInfo, MBUF *data, SOCKETINFO snd_flag)
{
	dave_bool ret = dave_false;

	SAFEZONEv5R(pCore->opt_pv, {
		if((pCore->use_flag == dave_true) && (pCore->wait_close == dave_false))
		{
			ret = dave_true;

			if(_socker_external_list_output(pCore, pIPInfo, data, snd_flag) == dave_false)
			{
				_socket_external_core_event(SOC_EVENT_SND, pCore);
			}
		}
	} );

	return ret;
}

static inline void
_socket_external_recv_event(SocketCore *pCore, SocketRawEvent *pEvent)
{
	if(pCore->type == SOCKET_TYPE_SERVER_FATHER)
	{
		if(pCore->NetInfo.type == TYPE_SOCK_STREAM)
		{
			SAFEZONEv5R(pCore->opt_pv,
				if((pCore->socket_external_index == pEvent->socket) && (pCore->os_socket == pEvent->os_socket))
				{
					_socket_external_accept(pCore);
				}
			);
		}
		else if(pCore->NetInfo.type == TYPE_SOCK_DGRAM)
		{
			SAFEZONEv5W(pCore->opt_pv,
				if((pCore->socket_external_index == pEvent->socket) && (pCore->os_socket == pEvent->os_socket))
				{
					_socket_external_recv(pCore, pEvent);
				}
			);
		}
		else
		{
			SOCKETABNOR("invalid recv event type! type:%d socket:%d/%d os_socket:%d type:%d",
				pCore->type,
				pCore->socket_external_index, pCore->socket_internal_index,
				pCore->os_socket, pCore->NetInfo.type);
		}
	}
	else if((pCore->type == SOCKET_TYPE_SERVER_CHILD)
		|| (pCore->type == SOCKET_TYPE_CLIENT)
		|| (pCore->type == SOCKET_TYPE_CLIENT_WAIT))
	{
		SAFEZONEv5R(pCore->opt_pv,
			if((pCore->socket_external_index == pEvent->socket) && (pCore->os_socket == pEvent->os_socket))
			{
				_socket_external_recv(pCore, pEvent);
			}
		);
	}
	else
	{
		SOCKETABNOR("invalid recv event! type:%d socket:%d/%d/%d",
			pCore->type,
			pCore->socket_external_index, pCore->socket_internal_index, pCore->os_socket);
	}
}

static inline void
_socket_external_list_event(SocketCore *pCore, SocketRawEvent *pEvent)
{
	SAFEZONEv5R(pCore->opt_pv,
		if((pCore->use_flag == dave_true)
			&& (pCore->wait_close == dave_false)
			&& ((pCore->socket_external_index == pEvent->socket) && (pCore->os_socket == pEvent->os_socket)))
		{
			_socker_external_list_output(pCore, NULL, NULL, SOCKETINFO_SND);
		}
	);
}

static inline void
_socket_external_link_event(SocketCore *pCore, SocketRawEvent *pEvent)
{
	if((pEvent->event <= SOC_EVENT_START) || (pEvent->event >= SOC_EVENT_MAX))
	{
		SOCKETLOG("invalid event:%d", pEvent->event);
		return;
	}

	switch(pEvent->event)
	{
		case SOC_EVENT_CONNECT:
				SAFEZONEv5W(pCore->opt_pv,
					if((pCore->socket_external_index == pEvent->socket) && (pCore->os_socket == pEvent->os_socket))
					{
						_socket_external_connect_event(pCore);
					}
				);
			break;
		case SOC_EVENT_CONNECT_FAIL:
				SAFEZONEv5W(pCore->opt_pv,
					if((pCore->socket_external_index == pEvent->socket) && (pCore->os_socket == pEvent->os_socket))
					{
						_socket_external_connect_failed_event(pCore);
					}
				);
			break;
		case SOC_EVENT_CLOSE:
				SAFEZONEv5W(pCore->opt_pv,
					if((pCore->socket_external_index == pEvent->socket) && (pCore->os_socket == pEvent->os_socket))
					{
						_socket_external_close_event(pCore);
					}
				);
			break;
		default:
				SOCKETABNOR("unknown event:%d", pEvent->event);
			break;
	}
}

// =====================================================================

void
socket_external_init(void)
{
	_socket_thread = self();

	_socket_external_socket_file_max();

	dave_os_socket_init(_socket_external_os_event);
}

void
socket_external_exit(void)
{
	dave_os_socket_exit();
}

SocketCore *
socket_external_creat_service(ThreadId src, SocNetInfo *pNetInfo, void *user_ptr)
{
	SocketCore *pCore;

	pCore = _socket_external_creat_service(src, pNetInfo, user_ptr);

	return pCore;
}

SocketCore *
socket_external_connect_service(ThreadId src, SocNetInfo *pNetInfo, void *user_ptr, SOCKETINFO *ConnectInfo)
{
	SocketCore *pCore;

	pCore = _socket_external_connect_service(src, pNetInfo, user_ptr, ConnectInfo);

	return pCore;
}

ErrCode
socket_external_close(ThreadId src, s32 socket)
{
	SocketCore *pCore;

	pCore = socket_core_find(socket);
	if(pCore != NULL)
	{
		_socket_external_core_event(SOC_EVENT_CLOSE, pCore);
	}

	return ERRCODE_OK;
}

dave_bool
socket_external_send(ThreadId src, s32 socket, IPBaseInfo *pIPInfo, MBUF *data, SOCKETINFO snd_flag)
{
	SocketCore *pCore;

	pCore = socket_core_find(socket);
	if(pCore == NULL)
	{
		SOCKETTRACE("%s socket:%d send:%d failed!",
			thread_name(src), socket, data->tot_len);
		return dave_false;
	}

	if((pCore->os_socket < 0) || (pCore->wait_close == dave_true))
	{
		SOCKETTRACE("socket:%d close! core socket:%d/%d/%d wait_close:%d",
			socket,
			pCore->socket_external_index, pCore->socket_internal_index, pCore->os_socket,
			pCore->wait_close);
		return dave_false;
	}

	if(pCore->owner != src)
	{
		SOCKETABNOR("%s write someone:%s socket:%d/%d/%d/%d",
			thread_name(src), thread_name(pCore->owner),
			pCore->socket_external_index, pCore->socket_internal_index,
			pCore->os_socket, socket);
		return dave_false;
	}

	return _socket_external_send_event(pCore, pIPInfo, data, snd_flag);
}

void
socket_external_notify(SocketNotify *pNotify)
{
	SocketCore *pCore = (SocketCore *)(pNotify->ptr);

	switch(pNotify->notify)
	{
		case SOCKETINFO_RAW_EVENT_RECV_LENGTH:
				_socket_external_recv_counter(pCore, pNotify->data);
			break;
		default:
			break;
	}
}

void
socket_external_event(SocketRawEvent *pEvent)
{
	SocketCore *pCore;

	pCore = socket_core_external_find(pEvent->socket, pEvent->os_socket);
	if((pCore == NULL) || (pCore->os_socket != pEvent->os_socket))
	{
		SOCKETTRACE("the os_socket:%d lost, event:%d!",
			pEvent->os_socket, pEvent->event);
		return;
	}

	if(pEvent->socket == INVALID_SOCKET_ID)
	{
		/*
		 * 如果socket为INVALID_SOCKET_ID，那么这个事件就来着系统，
		 * 在此默认给填充pCore的值，以方便在接下来的处理里面做
		 * (pCore->socket_external_index == pEvent->socket)
		 * 判断处理
		 */
		pEvent->socket = pCore->socket_external_index;
	}

	if(pEvent->event == SOC_EVENT_REV)
	{
		_socket_external_recv_event(pCore, pEvent);
	}
	else if(pEvent->event == SOC_EVENT_SND)
	{
		_socket_external_list_event(pCore, pEvent);
	}
	else
	{
		_socket_external_link_event(pCore, pEvent);
	}
}

#endif

