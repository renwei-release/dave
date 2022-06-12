/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_CLIENT
#include "dave_verno.h"
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "base_rxtx.h"
#include "log_log.h"

#define CFG_LOG_SERVER_IP_V4 "LOGSerIPV4"
#define CFG_LOG_SERVER_DOMAIN "LogServerDomain"

#define NUM_LOG_ONCE_SEND      (2048)
#define LOG_ONCE_SEND_BYTE_MAX (1500)
#define LOG_SEND_INTVL         (1000)
#define BDATA_TRACE_BUF_MAX	   (1024)

static void _log_stack_client_reconnect(TIMERID timer_id, ub thread_index);

static ThreadId _log_stack_client_thread = INVALID_THREAD_ID;
static TIMERID _log_stack_snd_log_timer_id = INVALID_TIMER_ID;
static s8 _log_product_name[2048];
static u16 _log_product_name_len;
static s8 _log_device_info[2048];
static u16 _log_device_info_len;
static ThreadId _socket_thread = INVALID_THREAD_ID;
static ThreadId _bdata_thread = INVALID_THREAD_ID;
static s32 _log_stack_client_socket = INVALID_SOCKET_ID;
static dave_bool _log_stack_client_snd_booting_message_flag = dave_false;

static void
_log_stack_client_disconnect(s32 socket)
{
	SocketDisconnectReq *pReq;

	if(socket != INVALID_SOCKET_ID)
	{
		pReq = thread_msg(pReq);

		pReq->socket = socket;

		id_msg(_socket_thread, SOCKET_DISCONNECT_REQ, pReq);
	}
}

static void
_log_stack_client_get_device_info(s8 *info_ptr, ub info_len)
{
	s8 host_name[128];
	u8 mac[DAVE_MAC_ADDR_LEN];

	dave_os_load_host_name(host_name, sizeof(host_name));

	dave_os_load_mac(mac);

	dave_snprintf(info_ptr, info_len, "%s-%02X%02X%02X%02X%02X%02X",
		host_name,
		mac[0], mac[1], mac[2],
		mac[3], mac[4], mac[5]);
}

static void
_log_stack_client_server_ip(u8 ip[DAVE_IP_V4_ADDR_LEN], u16 *port)
{
	s8 domain[1024];
	dave_bool domain_flag = dave_false;

	dave_memset(ip, 0x00, DAVE_IP_V4_ADDR_LEN);
	*port = LOG_SERVICE_PORT;

	if(cfg_get(CFG_LOG_SERVER_DOMAIN, (u8 *)domain, sizeof(domain)) == dave_true)
	{
		domain_flag = domainip(ip, port, domain);
		if(domain_flag == dave_true)
		{
			LOGDEBUG("load log server:%s->%s", domain, ipv4str(ip, *port));
		}
	}

	if(domain_flag == dave_false)
	{
		if(cfg_get(CFG_LOG_SERVER_IP_V4, ip, DAVE_IP_V4_ADDR_LEN) == dave_false)
		{
			ip[0] = 127; ip[1] = 0; ip[2] = 0; ip[3] = 1;
		}

		dave_snprintf(domain, sizeof(domain), "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], *port);
		cfg_set(CFG_LOG_SERVER_DOMAIN, domain, dave_strlen(domain));
	}

	*port = LOG_SERVICE_PORT;
}

static void
_log_stack_client_record_log(void)
{
	sb num_log;
	MBUF *data;
	ub index, len_index;
	ub log_len;
	s8 *frame;
	TraceLevel level;

    LOGDEBUG("%d", _log_stack_client_socket);

	if(_log_stack_client_socket == INVALID_SOCKET_ID)
	{
		return;
	}

	num_log = NUM_LOG_ONCE_SEND;

	while((num_log --) > 0)
	{
		data = dave_mmalloc(LOG_ONCE_SEND_BYTE_MAX);
		frame = (s8 *)dave_mptr(data);

		index = 0;
		dave_byte_8(frame[index++], frame[index++], _log_product_name_len);
		index += dave_memcpy(&frame[index], _log_product_name, _log_product_name_len);
		dave_byte_8(frame[index++], frame[index++], _log_device_info_len);
		index += dave_memcpy(&frame[index], _log_device_info, _log_device_info_len);
		len_index = index; log_len = 0;
		dave_byte_8(frame[index++], frame[index++], log_len);
		log_len = base_log_load(&frame[index], LOG_ONCE_SEND_BYTE_MAX-index, &level);
		if((log_len == 0) || (level >= TRACELEVEL_MAX))
		{
			dave_mfree(data);
			break;
		}

		dave_byte_8(frame[len_index], frame[len_index + 1], log_len);

		data->len = data->tot_len = index + log_len;

		if(rxtx_writes(_log_stack_client_socket, ORDER_CODE_LOG_RECORD, data) == dave_false)
			break;
	}
}

static void
_log_stack_client_send_booting_message(void)
{
	MBUF *data;
	ub index, len_index;
	s8 *frame;
	s8 *booting_message;
	ub booting_message_length;
	DateStruct date;

	if(_log_stack_client_snd_booting_message_flag == dave_true)
	{
		return;
	}

	if(_log_stack_client_socket == INVALID_SOCKET_ID)
	{
		return;
	}

	data = dave_mmalloc(LOG_ONCE_SEND_BYTE_MAX);

	index = 0;
	frame = dave_mptr(data);

	dave_byte_8(frame[index++], frame[index++], _log_product_name_len);
	index += dave_memcpy(&frame[index], _log_product_name, _log_product_name_len);
	dave_byte_8(frame[index++], frame[index++], _log_device_info_len);
	index += dave_memcpy(&frame[index], _log_device_info, _log_device_info_len);
	len_index = index; booting_message_length = 0;
	dave_byte_8(frame[index++], frame[index++], booting_message_length);

	booting_message = &frame[index];
	t_time_get_date(&date);
	booting_message_length = 0;
	booting_message_length += dave_sprintf(&booting_message[booting_message_length], "\n********************************\n");
	booting_message_length += dave_sprintf(&booting_message[booting_message_length], "%04d.%02d.%02d %02d:%02d:%02d booting system ...\n",
		date.year, date.month, date.day, date.hour, date.minute, date.second);
	booting_message_length += dave_sprintf(&booting_message[booting_message_length], "%s\n", dave_verno());
	booting_message_length += dave_sprintf(&booting_message[booting_message_length], "********************************\n");
	dave_byte_8(frame[len_index], frame[len_index+1], booting_message_length);
	index += booting_message_length;

	data->len = data->tot_len = index;

	if(rxtx_writes(_log_stack_client_socket, ORDER_CODE_LOG_RECORD, data) == dave_true)
	{
		_log_stack_client_snd_booting_message_flag = dave_true;
	}
	else
	{
		dave_mfree(data);
	}
}

static void
_log_stack_client_connect_rsp(MSGBODY *ptr)
{
	SocketConnectRsp *pRsp = (SocketConnectRsp *)(ptr->msg_body);

	LOGDEBUG("");

	switch(pRsp->ConnectInfo)
	{
		case SOCKETINFO_CONNECT_OK:
		case SOCKETINFO_CONNECT_WAIT:
				LOGDEBUG("socket:%d", pRsp->socket);
			break;
		default:
				_log_stack_client_socket = INVALID_SOCKET_ID;
				base_timer_creat("logclientrec", _log_stack_client_reconnect, 3000);
				LOGDEBUG("failed! (%d)", pRsp->ConnectInfo);
			break;
	}
}

static void
_log_stack_client_connect_req(void)
{
	SocketConnectReq *pReq = thread_msg(pReq);

	pReq->NetInfo.domain = DM_SOC_PF_RAW_INET;
	pReq->NetInfo.type = TYPE_SOCK_DGRAM;
	pReq->NetInfo.addr_type = NetAddrIPType;
	pReq->NetInfo.addr.ip.ver = IPVER_IPV4;
	_log_stack_client_server_ip(pReq->NetInfo.addr.ip.ip_addr, &(pReq->NetInfo.port));
	pReq->NetInfo.fixed_src_flag = NotFixedPort;
	pReq->NetInfo.enable_keepalive_flag = KeepAlive_disable;
	pReq->NetInfo.netcard_bind_flag = NetCardBind_disable;
	_socket_thread = thread_id(SOCKET_THREAD_NAME);

	LOGDEBUG("server ip:%s", ipv4str(pReq->NetInfo.addr.ip.ip_addr, pReq->NetInfo.port));

	id_event(_socket_thread, SOCKET_CONNECT_REQ, pReq, SOCKET_CONNECT_RSP, _log_stack_client_connect_rsp);
}

static void
_log_stack_client_reconnect(TIMERID timer_id, ub thread_index)
{
	base_timer_die(timer_id);

	if(_log_stack_client_socket == INVALID_SOCKET_ID)
	{
		_log_stack_client_connect_req();
	}
}

static void
_log_stack_client_timer_fun(TIMERID timer_id, ub thread_index)
{
	if(_log_stack_snd_log_timer_id == timer_id)
	{
		_log_stack_client_record_log();
	}
	else
	{
		base_timer_die(timer_id);
	}
}

static void
_log_stack_client_start_snd_timer(void)
{
	if(_log_stack_snd_log_timer_id == INVALID_TIMER_ID)
	{
		_log_stack_snd_log_timer_id = base_timer_creat("lsc", _log_stack_client_timer_fun, LOG_SEND_INTVL);
	}
}

static void
_log_stack_client_stop_snd_timer(void)
{
	if(_log_stack_snd_log_timer_id != INVALID_TIMER_ID)
		base_timer_die(_log_stack_snd_log_timer_id);
	_log_stack_snd_log_timer_id = INVALID_TIMER_ID;
}

static void
_log_stack_client_plugin(SocketPlugIn *pPlugIn)
{
	LOGDEBUG("socket:%d %d", _log_stack_client_socket, pPlugIn->child_socket);

	_log_stack_client_socket = pPlugIn->child_socket;

	build_rxtx(TYPE_SOCK_DGRAM, _log_stack_client_socket, pPlugIn->NetInfo.port);

	_log_stack_client_send_booting_message();

	_log_stack_client_start_snd_timer();
}

static void
_log_stack_client_plugout(SocketPlugOut *pPlugOut)
{
	LOGDEBUG("socket:%d %d", _log_stack_client_socket, pPlugOut->socket);

	if(_log_stack_client_socket == pPlugOut->socket)
	{
		clean_rxtx(_log_stack_client_socket);

		_log_stack_client_stop_snd_timer();

		if(base_power_state() == dave_true)
		{
			base_timer_creat("logclientrec", _log_stack_client_reconnect, 3000);
		}

		_log_stack_client_socket = INVALID_SOCKET_ID;
	}
}

static void
_log_stack_client_debug(DebugReq *pReq)
{
	if(pReq->msg[0] != '\0')
	{
		LOGLOG("%s", pReq->msg);
	}
}

static void
_log_stack_client_reboot(RESTARTREQMSG *pRestart)
{
	if(pRestart->times == 1)
	{
		if(_log_stack_client_socket != INVALID_SOCKET_ID)
		{
			_log_stack_client_disconnect(_log_stack_client_socket);
		}
	}
}

static void
_log_stack_client_cfg_update(CFGUpdate *pUpdate)
{
	if((dave_strcmp(pUpdate->cfg_name, CFG_LOG_SERVER_IP_V4) == dave_true)
		|| (dave_strcmp(pUpdate->cfg_name, CFG_LOG_SERVER_DOMAIN) == dave_true))
	{
		LOGDEBUG("config:%s update!", pUpdate->cfg_name);

		if(_log_stack_client_socket != INVALID_SOCKET_ID)
		{
			_log_stack_client_disconnect(_log_stack_client_socket);

			LOGDEBUG("wait _log_stack_client_plugout reconnect!");
		}
	}
}

static void
_log_stack_client_init(MSGBODY *msg)
{
	_log_stack_snd_log_timer_id = INVALID_TIMER_ID;

	dave_verno_product(dave_verno(), _log_product_name, sizeof(_log_product_name));
	_log_product_name_len = dave_strlen(_log_product_name);
	_log_stack_client_get_device_info(_log_device_info, sizeof(_log_device_info));
	_log_device_info_len = dave_strlen(_log_device_info);

	_socket_thread = thread_id(SOCKET_THREAD_NAME);
	_bdata_thread = INVALID_THREAD_ID;

	_log_stack_client_socket = INVALID_SOCKET_ID;

	_log_stack_client_snd_booting_message_flag = dave_false;

	_log_stack_client_connect_req();
}

static void
_log_stack_client_main(MSGBODY *msg)
{	
	switch((ub)msg->msg_id)
	{
		case MSGID_DEBUG_REQ:
				_log_stack_client_debug((DebugReq *)(msg->msg_body));
			break;
		case MSGID_RESTART_REQ:
		case MSGID_POWER_OFF:
				_log_stack_client_reboot((RESTARTREQMSG *)(msg->msg_body));
			break;
		case SOCKET_PLUGIN:
				_log_stack_client_plugin((SocketPlugIn *)(msg->msg_body));
			break;
		case SOCKET_PLUGOUT:
				_log_stack_client_plugout((SocketPlugOut *)(msg->msg_body));
			break;
		case MSGID_CFG_UPDATE:
				_log_stack_client_cfg_update((CFGUpdate *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_log_stack_client_exit(MSGBODY *msg)
{
	if(msg == NULL)
		return;

	_log_stack_client_stop_snd_timer();

	if(_log_stack_client_socket != INVALID_SOCKET_ID)
	{
		clean_rxtx(_log_stack_client_socket);
		_log_stack_client_socket = INVALID_SOCKET_ID;
	}
}

// =====================================================================

void
log_stack_client_init(void)
{
	_log_stack_client_thread = base_thread_creat(LOG_CLIENT_THREAD_NAME, 1, THREAD_THREAD_FLAG|THREAD_PRIVATE_FLAG, _log_stack_client_init, _log_stack_client_main, _log_stack_client_exit);
	if(_log_stack_client_thread == INVALID_THREAD_ID)
		base_restart(LOG_CLIENT_THREAD_NAME);
}

void
log_stack_client_exit(void)
{
	if(_log_stack_client_thread != INVALID_THREAD_ID)
		base_thread_del(_log_stack_client_thread);
	_log_stack_client_thread = INVALID_THREAD_ID;
}

#endif

