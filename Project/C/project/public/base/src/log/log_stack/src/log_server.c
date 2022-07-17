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
#include "log_log.h"

#define CFG_LOG_SERVER_PORT "LogServerPort"

#define TIME_COUNT_DEFAULT_VAL		3
#define CLOSE_LOG_FILE_TIME_OUT		60*1000
#define LOG_FILE_POOL_MAX			81920
#define MAX_OPEN_FILE_PER_PROCESS	1000 /* setrlimit */
#define LOG_DEVICE_NAME_LEN (6)

typedef struct {
	s8 file_name[128];
	sb file_id;
} LogFileMap;

typedef struct _HashNode {
	LogFileMap map;
	sb time_count;
	struct _HashNode *next;
} HashNode;

static ThreadId _log_stack_server_thread = INVALID_THREAD_ID;
static ThreadId _socket_thread = INVALID_THREAD_ID;
static s32 _log_stack_server_socket = INVALID_SOCKET_ID;
static TIMERID _close_log_file_timer_id = INVALID_TIMER_ID;
static HashNode *_hash_table[LOG_FILE_POOL_MAX];
static sb _open_file_count = 0;

static void
_log_stack_server_close_all_log_file(void)
{
	ub index;
	HashNode *node = NULL, *next = NULL;

	for(index=0; index<LOG_FILE_POOL_MAX; index++)
	{
		node = _hash_table[index];
		while (node != NULL)
		{
			dave_os_file_close(node->map.file_id);
			-- _open_file_count;
			if(_open_file_count < 0)
			{
				LOGABNOR("arithmetic error");
			}
			next = node->next;
			dave_free(node);
			node = next;
		}
		_hash_table[index] = NULL;
	}
}

static void
_log_stack_server_close_log_file(void)
{
	ub index;
	HashNode *pre, *cur = NULL, *tmp = NULL;

	for(index=0; index<LOG_FILE_POOL_MAX; index++)
	{
		pre = cur = _hash_table[index];
		while(cur != NULL)
		{
			cur->time_count--;
			if(cur->time_count <= 0)
			{
				tmp = cur;

				dave_os_file_close(cur->map.file_id);

				-- _open_file_count;

				if(_open_file_count < 0)
				{
					LOGABNOR("arithmetic error");
				}

				if (cur == _hash_table[index])
				{
					_hash_table[index] = cur->next;
					pre = cur = cur->next;
				}
				else
				{
					pre->next = cur->next;
					cur = cur->next;
				}

				dave_free(tmp);
			}
			else
			{
				pre = cur;
				cur = cur->next;
			}
		}
	}
}

static inline HashNode *
_log_stack_server_malloc_hash_node(s8 *file_name, sb file_id)
{
	HashNode *node;

	node = dave_ralloc(sizeof(HashNode));
	if (node != NULL)
	{
		dave_strcpy(node->map.file_name, file_name, 128);
		node->map.file_id = file_id;
		node->time_count = TIME_COUNT_DEFAULT_VAL;
		node->next = NULL;
	}

	return node;
}

static inline ub
_log_stack_server_build_hash_key(s8 *file_name)
{
	ub key = 0;
	s8 *ptr = file_name;

	while(*ptr != '\0')
	{
		key += *ptr;
		ptr++;
	}

	return (key % LOG_FILE_POOL_MAX);
}

static inline sb
_log_stack_server_get_log_file_from_pool(s8 *file_name)
{
	HashNode *node;
	ub key = _log_stack_server_build_hash_key(file_name);

	node = _hash_table[key];

	while(node != NULL)
	{
		if(dave_strcmp(node->map.file_name, file_name) == dave_true)
		{
			node->time_count = TIME_COUNT_DEFAULT_VAL;
			return node->map.file_id;
		}

		node = node->next;
	}

	return -1;
}

static inline void
_log_stack_server_insert_log_file_to_pool(s8 *file_name, sb file_id)
{
	HashNode *node = NULL, *new_node = NULL;
	u16 key = _log_stack_server_build_hash_key(file_name);

	node = _hash_table[key];
	new_node = _log_stack_server_malloc_hash_node(file_name, file_id);
	if (new_node != NULL)
	{
		new_node->next = node;
		_hash_table[key] = new_node;
	}
}

static void
_log_stack_server_close_log_file_timer_fun(TIMERID timer_id, ub thread_index)
{
	_log_stack_server_close_log_file();
}

static inline void
_log_stack_server_build_record_file_name(s8 *file_name_ptr, ub file_name_len, s8 *product_name, s8 *device_info)
{
	DateStruct date = t_time_get_date(NULL);

	dave_snprintf(file_name_ptr, file_name_len, "%s/%04d%02d%02d/%s",
		product_name,
		date.year, date.month, date.day,
		device_info);
}

static const s8 _content_separator[][2] = {
	{'(', ')'},
	{'<', '>'},
	{'{', '}'},
	{'[', ']'},
	{0, 0}
};

static inline dave_bool
_log_stack_server_is_start_separator(s8 content)
{
	ub separator_index;

	for(separator_index=0; separator_index<128; separator_index++)
	{
		if(_content_separator[separator_index][0] == 0)
			break;

		if(_content_separator[separator_index][0] == content)
			return dave_true;
	}

	return dave_false;
}

static inline dave_bool
_log_stack_server_is_end_separator(s8 content)
{
	ub separator_index;

	for(separator_index=0; separator_index<128; separator_index++)
	{
		if(_content_separator[separator_index][1] == 0)
			break;

		if(_content_separator[separator_index][1] == content)
			return dave_true;
	}

	return dave_false;
}

static inline ub
_log_stack_server_load_record(s8 *separator, s8 **record_ptr, ub *record_len, s8 *content_ptr, ub content_len)
{
	ub content_index;
	s8 *load_record_ptr;
	ub load_record_len;

	*separator = '(';
	*record_ptr = NULL;
	*record_len = 0;

	content_index = 0;

	while(content_index < content_len)
	{
		if(_log_stack_server_is_start_separator(content_ptr[content_index]) == dave_true)
		{
			*separator = content_ptr[content_index];
			content_index ++;
			break;
		}
		content_index ++;
	}
	if(content_index >= content_len)
	{
		return content_index;
	}

	load_record_ptr = &content_ptr[content_index];
	load_record_len = 0;

	while(content_index < content_len)
	{
		if(_log_stack_server_is_end_separator(content_ptr[content_index]) == dave_true)
		{
			*record_ptr = load_record_ptr;
			*record_len = load_record_len;

			content_index ++;
			break;
		}

		load_record_len ++;
		content_index ++;
	}

	if(*record_ptr == NULL)
		return 0;

	return content_index;
}

static inline void
_log_stack_server_insert_receive_date(void *pJson)
{
	DateStruct date;
	s8 date_ptr[128];
	ub date_len;

	t_time_get_date(&date);

	date_len = dave_snprintf(date_ptr, sizeof(date_ptr),
		"%04d.%02d.%02d %02d:%02d:%02d",
		date.year, date.month,
		date.day, date.hour,
		date.minute, date.second);

	dave_json_add_str_len(pJson, "receive_date", date_ptr, date_len);
}

static inline void
_log_stack_server_insert_fun_and_line(void *pJson, s8 *record_ptr, ub record_len)
{
	ub record_index;

	for(record_index=0; record_index<record_len; record_index++)
	{
		if(record_ptr[record_index] == ':')
		{
			dave_json_add_str_len(pJson, "function", record_ptr, record_index);
			record_index ++;
			break;
		}
	}

	if(record_index >= record_len)
		return;

	dave_json_add_str_len(pJson, "line", &record_ptr[record_index], record_len-record_index);
}

static inline ub
_log_stack_server_insert_content(void *pJson, s8 *content_ptr, ub content_len)
{
	s8 separator;
	s8 *record_ptr;
	ub record_len;
	ub content_index;

	content_index = 0;

	content_index += _log_stack_server_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	dave_json_add_str_len(pJson, "version", record_ptr, record_len);

	content_index += _log_stack_server_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	dave_json_add_str_len(pJson, "build_date", record_ptr, record_len);

	content_index += _log_stack_server_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	if(separator == '{')
		dave_json_add_str_len(pJson, "serial", record_ptr, record_len);
	else
		dave_json_add_str_len(pJson, "service", record_ptr, record_len);

	content_index += _log_stack_server_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	if(separator == '[')
	{
		dave_json_add_str_len(pJson, "service", record_ptr, record_len);
	}
	if(separator == '<')
	{
		_log_stack_server_insert_fun_and_line(pJson, record_ptr, record_len);
	}

	content_index += _log_stack_server_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	if(separator == '<')
	{
		_log_stack_server_insert_fun_and_line(pJson, record_ptr, record_len);
	}

	return content_index;
}

static inline void
_log_stack_server_save_log(sb file_id, TraceLevel level, s8 *content_ptr, ub content_len)
{
	void *pJson;
	sb file_len;
	ub process_len;
	s8 *json_str;
	ub json_len;

	pJson = dave_json_malloc();

	_log_stack_server_insert_receive_date(pJson);
	dave_json_add_str(pJson, "level", t_auto_TraceLevel_str(level));
	process_len = _log_stack_server_insert_content(pJson, content_ptr, content_len);
	if(process_len == 0)
	{
		dave_json_add_str_len(pJson, "log_body", content_ptr, content_len);
	}
	else if(process_len < content_len)
	{
		dave_json_add_str_len(pJson, "log_body", &content_ptr[process_len], content_len-process_len);
	}
	dave_json_add_str_len(pJson, "content", content_ptr, content_len);

	file_len = dave_os_file_len(NULL, file_id);
	if(file_len < 0)
	{
		file_len = 0;
	}

	json_str = dave_json_to_string(pJson, &json_len);

	file_len += dave_os_file_save(file_id, (ub)file_len, json_len, (u8 *)json_str);
	dave_os_file_save(file_id, (ub)file_len, 1, (u8 *)"\n");

	dave_json_free(pJson);
}

static void
_log_stack_server_save_file(s8 *log_file_name, TraceLevel level, s8 *content_ptr, ub content_len)
{
	sb file_id;

	if((file_id = _log_stack_server_get_log_file_from_pool(log_file_name)) > 0)
	{
		_log_stack_server_save_log(file_id, level, content_ptr, content_len);
	}
	else
	{
		file_id = dave_os_file_open(CREAT_READ_WRITE_FLAG, log_file_name);
		if(file_id >= 0)
		{
			++ _open_file_count;

			_log_stack_server_save_log(file_id, level, content_ptr, content_len);

			if(_open_file_count < MAX_OPEN_FILE_PER_PROCESS)
			{
				_log_stack_server_insert_log_file_to_pool(log_file_name, file_id);
			}
			else
			{
				dave_os_file_close(file_id);

				-- _open_file_count;

				if(_open_file_count < 0)
				{
					LOGABNOR("arithmetic error");
				}
			}
		}
		else
		{
			LOGABNOR("save log:%s to file:%s failed!", content_ptr, log_file_name);
		}
	}
}

static void
_log_stack_server_log_record(ub frame_len, u8 *frame)
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

	_log_stack_server_build_record_file_name(log_file_name, sizeof(log_file_name), product_name, device_info);

	_log_stack_server_save_file(log_file_name, TRACELEVEL_LOG, (s8 *)(&frame[frame_index]), content_len);
}

static void
_log_stack_server_log_record_v2(ub frame_len, u8 *frame)
{
	ub frame_index;
	s8 log_file_name[256];
	s8 product_name[256];
	u16 product_name_len;
	s8 device_info[256];
	u16 device_info_len;
	TraceLevel level;
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
	
	dave_byte_16(level, frame[frame_index++], frame[frame_index++]);
	dave_byte_16(content_len, frame[frame_index++], frame[frame_index++]);

	if(content_len > (frame_len - frame_index))
	{
		LOGABNOR("invalid content_len:%d frame_len:%d frame_index:%d",
			content_len, frame_len, frame_index);
		content_len = frame_len - frame_index;
	}

	_log_stack_server_build_record_file_name(log_file_name, sizeof(log_file_name), product_name, device_info);

	_log_stack_server_save_file(log_file_name, level, (s8 *)(&frame[frame_index]), content_len);
}

static void
_log_stack_server_rx(void *param, s32 socket, IPBaseInfo *pInfo, FRAMETYPE ver_type, ORDER_CODE order_id, ub frame_len, u8 *frame)
{
	LOGDEBUG("order_id:%x ver_type:%d frame_len:%d",
		order_id, ver_type, frame_len);

	switch(order_id)
	{
		case ORDER_CODE_LOG_RECORD:
				_log_stack_server_log_record(frame_len, frame);
			break;
		case ORDER_CODE_LOG_RECORD_V2:
				_log_stack_server_log_record_v2(frame_len, frame);
			break;
		default:
			break;
	}
}

static void
_log_stack_server_bind_rsp(MSGBODY *ptr)
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
_log_stack_server_bind_req(void)
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

	id_event(_socket_thread, SOCKET_BIND_REQ, pReq, SOCKET_BIND_RSP, _log_stack_server_bind_rsp);
}

static void
_log_stack_server_plugin(SocketPlugIn *pPlugIn)
{
	LOGDEBUG("socket:%d", pPlugIn->father_socket);

	_log_stack_server_socket = pPlugIn->father_socket;

	build_rxtx(TYPE_SOCK_DGRAM, pPlugIn->father_socket, pPlugIn->NetInfo.port);
}

static void
_log_stack_server_plugout(SocketPlugOut *pPlugOut)
{
	LOGDEBUG("socket:%d", pPlugOut->socket);

	clean_rxtx(pPlugOut->socket);
	_log_stack_server_socket = INVALID_SOCKET_ID;
}

static void
_log_stack_server_reboot(RESTARTREQMSG *pRestart)
{
	SocketDisconnectReq disconnect;

	if(pRestart->times == 1)
	{
		if(_log_stack_server_socket != INVALID_SOCKET_ID)
		{
			disconnect.socket = _log_stack_server_socket;

			id_msg(_socket_thread, SOCKET_DISCONNECT_REQ, &disconnect);
		}
	}
}

static void
_log_stack_server_init(MSGBODY *msg)
{
	ub index;

	_socket_thread = thread_id(SOCKET_THREAD_NAME);
	_log_stack_server_socket = INVALID_SOCKET_ID;
	for(index=0; index<LOG_FILE_POOL_MAX; index++)
	{
		_hash_table[index] = NULL;
	}
	_close_log_file_timer_id = base_timer_creat("CLOSE LOG FILE", _log_stack_server_close_log_file_timer_fun, CLOSE_LOG_FILE_TIME_OUT);

	_log_stack_server_bind_req();
}

static void
_log_stack_server_main(MSGBODY *msg)
{	
	switch((ub)msg->msg_id)
	{
		case MSGID_RESTART_REQ:
		case MSGID_POWER_OFF:
				_log_stack_server_reboot((RESTARTREQMSG *)(msg->msg_body));
			break;
		case SOCKET_PLUGIN:
				_log_stack_server_plugin((SocketPlugIn *)msg->msg_body);
			break;
		case SOCKET_PLUGOUT:
				_log_stack_server_plugout((SocketPlugOut *)msg->msg_body);
			break;
		case SOCKET_RAW_EVENT:
				rxtx_event((SocketRawEvent *)(msg->msg_body), _log_stack_server_rx, NULL);
			break;
		default:
			break;
	}
}

static void
_log_stack_server_exit(MSGBODY *msg)
{
	if(_close_log_file_timer_id != INVALID_TIMER_ID)
		base_timer_die(_close_log_file_timer_id);
	_close_log_file_timer_id = INVALID_TIMER_ID;

	_log_stack_server_close_all_log_file();

	if(_log_stack_server_socket != INVALID_SOCKET_ID)
	{
		clean_rxtx(_log_stack_server_socket);
		_log_stack_server_socket = INVALID_SOCKET_ID;
	}
}

// =====================================================================

void
log_stack_server_init(void)
{
	_log_stack_server_thread = base_thread_creat(LOG_SERVER_THREAD_NAME, 1, THREAD_THREAD_FLAG|THREAD_PRIVATE_FLAG, _log_stack_server_init, _log_stack_server_main, _log_stack_server_exit);
	if(_log_stack_server_thread == INVALID_THREAD_ID)
		base_restart(LOG_SERVER_THREAD_NAME);
}

void
log_stack_server_exit(void)
{
	if(_log_stack_server_thread != INVALID_THREAD_ID)
		base_thread_del(_log_stack_server_thread);
	_log_stack_server_thread = INVALID_THREAD_ID;
}

#endif

