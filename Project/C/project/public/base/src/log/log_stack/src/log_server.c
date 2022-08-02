/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include "dave_base.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_verno.h"
#include "base_rxtx.h"
#include "log_save.h"
#include "log_log.h"

#define CFG_LOG_SERVER_PORT "LogServerPort"

#define TIME_COUNT_DEFAULT_VAL		3
#define CLOSE_LOG_FILE_TIME_OUT		60*1000
#define LOG_FILE_POOL_MAX			81920
#define MAX_OPEN_FILE_PER_PROCESS	1000 /* setrlimit */
#define LOG_DEVICE_NAME_LEN (6)

static ThreadId _log_server_thread = INVALID_THREAD_ID;
static ThreadId _socket_thread = INVALID_THREAD_ID;
static s32 _log_server_socket = INVALID_SOCKET_ID;

static void
_log_server_build_file_name(s8 *file_name_ptr, ub file_name_len, s8 *project_name, s8 *device_info)
{
	DateStruct date = t_time_get_date(NULL);

	if(device_info != NULL)
	{
		dave_snprintf(file_name_ptr, file_name_len, "%s/%04d%02d%02d/%s",
			project_name,
			date.year, date.month, date.day,
			device_info);
	}
	else
	{
		dave_snprintf(file_name_ptr, file_name_len, "%s/%04d%02d%02d/%02d/%02d",
			project_name,
			date.year, date.month, date.day,
			date.hour, date.minute);
	}
}

static void
_log_server_log_record(ub frame_len, u8 *frame)
{
	ub frame_index;
	s8 log_file_name[256];
	s8 product_name[256];
	u16 product_name_len;
	s8 device_info[256];
	u16 device_info_len;
	u16 content_len;

	frame_index = 0;

	dave_byte_16(product_name_len, frame[frame_index++], frame[frame_index++]);
	frame_index += dave_memcpy(product_name, &frame[frame_index], product_name_len);
	if(product_name_len < sizeof(product_name))
	{
		product_name[product_name_len] = '\0';
	}

	dave_byte_16(device_info_len, frame[frame_index++], frame[frame_index++]);
	frame_index += dave_memcpy(device_info, &frame[frame_index], device_info_len);
	if(device_info_len < sizeof(device_info))
	{
		device_info[device_info_len] = '\0';
	}

	dave_byte_16(content_len, frame[frame_index++], frame[frame_index++]);

	if(content_len > (frame_len - frame_index))
	{
		LOGABNOR("invalid content_len:%d frame_len:%d frame_index:%d",
			content_len, frame_len, frame_index);
		content_len = frame_len - frame_index;
	}

	_log_server_build_file_name(log_file_name, sizeof(log_file_name), product_name, device_info);

	log_save_log_file(log_file_name, TRACELEVEL_LOG, (s8 *)(&frame[frame_index]), content_len);
}

static void
_log_server_log_record_v2(ub frame_len, u8 *frame)
{
	ub frame_index;
	s8 product_name[256];
	u16 product_name_len;
	s8 device_info[256];
	u16 device_info_len;
	TraceLevel level;
	u16 log_len;
	s8 log_file_name[256];

	frame_index = 0;

	dave_byte_16(product_name_len, frame[frame_index++], frame[frame_index++]);
	frame_index += dave_memcpy(product_name, &frame[frame_index], product_name_len);
	if(product_name_len < sizeof(product_name))
	{
		product_name[product_name_len] = '\0';
	}
	dave_byte_16(device_info_len, frame[frame_index++], frame[frame_index++]);
	frame_index += dave_memcpy(device_info, &frame[frame_index], device_info_len);
	if(device_info_len < sizeof(device_info))
	{
		device_info[device_info_len] = '\0';
	}
	dave_byte_16(level, frame[frame_index++], frame[frame_index++]);
	dave_byte_16(log_len, frame[frame_index++], frame[frame_index++]);
	if(log_len > (frame_len - frame_index))
	{
		LOGABNOR("invalid content_len:%d frame_len:%d frame_index:%d",
			log_len, frame_len, frame_index);
		log_len = frame_len - frame_index;
	}

	_log_server_build_file_name(log_file_name, sizeof(log_file_name), product_name, device_info);

	log_save_log_file(log_file_name, level, (s8 *)(&frame[frame_index]), log_len);
}

static void
_log_server_log_chain(ub frame_len, u8 *frame)
{
	ub frame_index;
	s8 chain_name[256];
	u16 chain_name_len;
	s8 device_info[256];
	u16 device_info_len;
	s8 service_verno[256];
	u16 service_verno_len;
	u16 chain_len;
	s8 log_file_name[256];

	frame_index = 0;

	dave_byte_16(chain_name_len, frame[frame_index++], frame[frame_index++]);
	frame_index += dave_memcpy(chain_name, &frame[frame_index], chain_name_len);
	if(chain_name_len < sizeof(chain_name))
	{
		chain_name[chain_name_len] = '\0';
	}
	dave_byte_16(device_info_len, frame[frame_index++], frame[frame_index++]);
	frame_index += dave_memcpy(device_info, &frame[frame_index], device_info_len);
	if(device_info_len < sizeof(device_info))
	{
		device_info[device_info_len] = '\0';
	}
	dave_byte_16(service_verno_len, frame[frame_index++], frame[frame_index++]);
	frame_index += dave_memcpy(service_verno, &frame[frame_index], service_verno_len);
	if(service_verno_len < sizeof(service_verno))
	{
		service_verno[service_verno_len] = '\0';
	}
	dave_byte_8_32(chain_len, frame[frame_index++], frame[frame_index++], frame[frame_index++], frame[frame_index++]);

	_log_server_build_file_name(log_file_name, sizeof(log_file_name), chain_name, NULL);

	log_save_chain_file(log_file_name, device_info, service_verno, (s8 *)(&frame[frame_index]), chain_len);
}

static void
_log_server_rx(void *param, s32 socket, IPBaseInfo *pInfo, FRAMETYPE ver_type, ORDER_CODE order_id, ub frame_len, u8 *frame)
{
	LOGDEBUG("order_id:%x ver_type:%d frame_len:%d",
		order_id, ver_type, frame_len);

	switch(order_id)
	{
		case ORDER_CODE_LOG_RECORD:
				_log_server_log_record(frame_len, frame);
			break;
		case ORDER_CODE_LOG_RECORD_V2:
				_log_server_log_record_v2(frame_len, frame);
			break;
		case ORDER_CODE_LOG_CHAIN:
				_log_server_log_chain(frame_len, frame);
			break;
		default:
			break;
	}
}

static void
_log_server_bind_rsp(MSGBODY *ptr)
{
	SocketBindRsp *pRsp = (SocketBindRsp *)(ptr->msg_body);

	switch(pRsp->BindInfo)
	{
		case SOCKETINFO_BIND_OK:
				LOGDEBUG("server log bind OK! (port:%d socket: %d)", pRsp->NetInfo.port, pRsp->socket);
			break;
		default:
				LOGABNOR("server log bind failed! (%d)", pRsp->BindInfo);
			break;
	}
}

static void
_log_server_bind_req(void)
{
	SocketBindReq *pReq = thread_msg(pReq);

	pReq->NetInfo.domain = DM_SOC_PF_RAW_INET;
	pReq->NetInfo.type = TYPE_SOCK_DGRAM;
	pReq->NetInfo.addr_type = NetAddrIPType;
	pReq->NetInfo.addr.ip.ver = IPVER_IPV4;
	dave_memset(pReq->NetInfo.addr.ip.ip_addr, 0x00, 16);
	pReq->NetInfo.port = cfg_get_ub(CFG_LOG_SERVER_PORT);
	if(pReq->NetInfo.port == 0)
	{
		pReq->NetInfo.port = LOG_SERVICE_PORT;
		cfg_set_ub(CFG_LOG_SERVER_PORT, pReq->NetInfo.port);
	}
	pReq->NetInfo.fixed_src_flag = NotFixedPort;
	pReq->NetInfo.enable_keepalive_flag = KeepAlive_disable;
	pReq->NetInfo.netcard_bind_flag = NetCardBind_disable;

	LOGDEBUG("%s", ipv4str(pReq->NetInfo.addr.ip.ip_addr, pReq->NetInfo.port));

	id_event(_socket_thread, SOCKET_BIND_REQ, pReq, SOCKET_BIND_RSP, _log_server_bind_rsp);
}

static void
_log_server_plugin(SocketPlugIn *pPlugIn)
{
	LOGDEBUG("socket:%d", pPlugIn->father_socket);

	_log_server_socket = pPlugIn->father_socket;

	build_rxtx(TYPE_SOCK_DGRAM, pPlugIn->father_socket, pPlugIn->NetInfo.port);
}

static void
_log_server_plugout(SocketPlugOut *pPlugOut)
{
	LOGDEBUG("socket:%d", pPlugOut->socket);

	clean_rxtx(pPlugOut->socket);
	_log_server_socket = INVALID_SOCKET_ID;
}

static void
_log_server_reboot(RESTARTREQMSG *pRestart)
{
	SocketDisconnectReq disconnect;

	if(pRestart->times == 1)
	{
		if(_log_server_socket != INVALID_SOCKET_ID)
		{
			disconnect.socket = _log_server_socket;

			id_msg(_socket_thread, SOCKET_DISCONNECT_REQ, &disconnect);
		}
	}
}

static void
_log_server_init(MSGBODY *msg)
{
	_socket_thread = thread_id(SOCKET_THREAD_NAME);
	_log_server_socket = INVALID_SOCKET_ID;

	_log_server_bind_req();

	log_save_init();
}

static void
_log_server_main(MSGBODY *msg)
{	
	switch((ub)msg->msg_id)
	{
		case MSGID_RESTART_REQ:
		case MSGID_POWER_OFF:
				_log_server_reboot((RESTARTREQMSG *)(msg->msg_body));
			break;
		case SOCKET_PLUGIN:
				_log_server_plugin((SocketPlugIn *)msg->msg_body);
			break;
		case SOCKET_PLUGOUT:
				_log_server_plugout((SocketPlugOut *)msg->msg_body);
			break;
		case SOCKET_RAW_EVENT:
				rxtx_event((SocketRawEvent *)(msg->msg_body), _log_server_rx, NULL);
			break;
		default:
			break;
	}
}

static void
_log_server_exit(MSGBODY *msg)
{
	log_save_exit();

	if(_log_server_socket != INVALID_SOCKET_ID)
	{
		clean_rxtx(_log_server_socket);
		_log_server_socket = INVALID_SOCKET_ID;
	}
}

// =====================================================================

void
log_server_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_log_server_thread = base_thread_creat(LOG_SERVER_THREAD_NAME, thread_number, THREAD_THREAD_FLAG|THREAD_PRIVATE_FLAG, _log_server_init, _log_server_main, _log_server_exit);
	if(_log_server_thread == INVALID_THREAD_ID)
		base_restart(LOG_SERVER_THREAD_NAME);
}

void
log_server_exit(void)
{
	if(_log_server_thread != INVALID_THREAD_ID)
		base_thread_del(_log_server_thread);
	_log_server_thread = INVALID_THREAD_ID;
}

#endif

