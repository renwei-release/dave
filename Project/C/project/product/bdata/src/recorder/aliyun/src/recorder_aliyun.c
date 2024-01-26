/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BDATA__
#include "log_producer_client.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_os.h"
#include "bdata_log.h"

#define ALIYUN_LOG_CONFIG_FILE (s8 *)"/dave/tools/aliyun/log/log_config.json"

static log_producer *_aliyun_log_producer = NULL;
static log_producer_client *_aliyun_client = NULL;
static s8 _local_mac_str[DAVE_MAC_ADDR_LEN * 2 +1];
static s8 _local_ip_v4_str[DAVE_IP_V4_ADDR_LEN * 2 + 1];
static s8 _local_ip_v6_str[DAVE_IP_V6_ADDR_LEN * 2 + 1];
static s8 _local_hostname[DAVE_NORMAL_NAME_LEN];
static void *_aliyun_access_kv = NULL;

static void
_aliyun_log_send_done(const char *config_name, log_producer_result result, size_t log_bytes, size_t compressed_bytes, const char *req_id, const char *error_message, const unsigned char *raw_buffer)
{
	if(result != LOG_PRODUCER_OK)
	{
		BDLOG("config_name:%s error:%d/%s", config_name, result, error_message);
	}
}

static void
_aliyun_log_info_load(s8 *mac_str, s8 *ip_v4_str, s8 *ip_v6_str, s8 *hostname)
{
	u8 mac[DAVE_MAC_ADDR_LEN];
	u8 ip_v4[DAVE_IP_V4_ADDR_LEN];
	u8 ip_v6[DAVE_IP_V6_ADDR_LEN];
	s8 str_temp[128];

	dave_os_load_mac(mac);
	dave_sprintf(mac_str, "%s", macstr(mac));
	dave_os_load_ip(ip_v4, ip_v6);
	dave_sprintf(ip_v4_str, "%s", ipv4str(ip_v4, 0));
	dave_sprintf(ip_v6_str, "%s", ipstr(ip_v6, 6, str_temp, sizeof(str_temp)));
	dave_os_load_host_name(hostname, DAVE_NORMAL_NAME_LEN);
}

static ub
_aliyun_log_setup_conf(s8 *conf_buf, ub conf_buf_len, s8 *project, s8 *logstore, s8 *access_id, s8 *access_key)
{
	ub conf_buf_index;

	conf_buf_index = 0;

	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "{\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"send_thread_count\" : 4,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"endpoint\" : \"cn-hongkong.log.aliyuncs.com\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"project\" : \"%s\",\n", project);
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"logstore\" : \"%s\",\n", logstore);
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"access_id\" : \"%s\",\n", access_id);
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"access_key\" : \"%s\",\n", access_key);
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"name\" : \"root\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"topic\" : \"topic_test\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"tags\" : {\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "        \"tag_key\" : \"tag_value\"\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    },\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"level\" : \"INFO\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"priority\" : \"NORMAL\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"package_timeout_ms\" : 3000,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"log_count_per_package\" : 4096,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"log_bytes_per_package\" : 3000000,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"max_buffer_bytes\" : 100000000,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"debug_open\" : 0,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"debug_stdout\" : 0,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"debug_log_path\" : \"./log_debug.log\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"max_debug_logfile_count\" : 5,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"max_debug_logfile_size\" : 1000000,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    \"sub_appenders\" : {\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "        \"error\" : {\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"endpoint\" : \"cn-hongkong.log.aliyuncs.com\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"project\" : \"%s\",\n", project);
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"logstore\" : \"%s\",\n", logstore);
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"access_id\" : \"%s\",\n", access_id);
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"access_key\" : \"%s\",\n", access_key);
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"name\" : \"error\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"topic\" : \"topic_xxx\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"level\" : \"INFO\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"priority\" : \"HIGH\",\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"package_timeout_ms\" : 3000,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"log_count_per_package\" : 4096,\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "            \"max_buffer_bytes\" : 100000000\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "        }\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "    }\n");
	conf_buf_index += dave_sprintf(&conf_buf[conf_buf_index], "}\n");

	return conf_buf_index;
}

static dave_bool
_aliyun_log_update_conf_file(s8 *project, s8 *logstore, s8 *access_id, s8 *access_key)
{
	ub conf_buf_len = 4096;
	s8 *conf_buf;
	dave_bool ret;

	conf_buf = dave_malloc(conf_buf_len);

	conf_buf_len = _aliyun_log_setup_conf(conf_buf, conf_buf_len, project, logstore, access_id, access_key);

	dave_os_file_delete(DIRECT_FLAG, ALIYUN_LOG_CONFIG_FILE);

	ret = dave_os_file_write(DIRECT_FLAG|CREAT_WRITE_FLAG, ALIYUN_LOG_CONFIG_FILE, 0, conf_buf_len, (u8 *)conf_buf);

	dave_free(conf_buf);

	return ret;
}

static dave_bool
_aliyun_log_init(s8 *project, s8 *logstore, s8 *access_id, s8 *access_key)
{
	if(log_producer_env_init() == LOG_PRODUCER_OK)
	{
		_aliyun_log_update_conf_file(project, logstore, access_id, access_key);
	
		_aliyun_log_producer = create_log_producer_by_config_file((const char *)ALIYUN_LOG_CONFIG_FILE, _aliyun_log_send_done);
		if(_aliyun_log_producer != NULL)
		{
			_aliyun_client = get_log_producer_client(_aliyun_log_producer, NULL);

			if(_aliyun_client != NULL)
			{
				BDLOG("connect to aliyun log server! version:%s", LOG_API_VERSION);

				_aliyun_log_info_load(_local_mac_str, _local_ip_v4_str, _local_ip_v6_str, _local_hostname);

				return dave_true;
			}
		}
	}

	BDLOG("project:%s logstore:%s access_id:%s access_key:%s connecting failed!", project, logstore, access_id, access_key);
	return dave_false;
}

static void
_aliyun_log_exit(void)
{
	if(_aliyun_log_producer != NULL)
	{
		destroy_log_producer(_aliyun_log_producer);
		_aliyun_log_producer = NULL;
	}

	_aliyun_client = NULL;

	log_producer_env_destroy();
}

static dave_bool
_aliyun_log_info(s8 *log_file, s8 *log_ptr, ub log_len)
{
	DateStruct date;
	s8 date_str[128];
	log_producer_result result = LOG_PRODUCER_OK;

	if(_aliyun_client != NULL)
	{
		t_time_get_date(&date);

		dave_snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d %02d:%02d:%02d",
			date.year, date.month, date.day,
			date.hour, date.minute, date.second);

		LOG_PRODUCER_INFO(_aliyun_client,
			"index", log_file,
			"date", date_str,
			"host-mac", _local_mac_str,
			"host-ip-v4", _local_ip_v4_str,
			"host-ip-v6", _local_ip_v6_str,
			"host-name", _local_hostname,
			"json", log_ptr);

		if(result == LOG_PRODUCER_OK)
		{
			return dave_true;
		}
		else
		{
			BDLOG("aliyun log failed:%d", result);
		}
	}
	else
	{
		BDLOG("aliyun client is NULL!");
	}

	return dave_false;
}

static void
_aliyun_log_free_access(void)
{
	kv_free(_aliyun_access_kv, NULL);
	_aliyun_access_kv = NULL;
}

static void
_aliyun_log_malloc_access(void)
{
	s8 *cfg_key_name = "AliyunLogAccessLogFile";
	s8 *cfg_default_value = "['aliyun-chatbot-record', 'aliyun-chatbot-statement']";
	s8 temp_str[1024];
	void *pJson;
	ub array_len, array_index;
	s8 *log_file;

	if(_aliyun_access_kv != NULL)
	{
		_aliyun_log_free_access();
		_aliyun_access_kv = NULL;
	}

	_aliyun_access_kv = kv_malloc("aliyun-access", 0, NULL);

	cfg_get_str(cfg_key_name, temp_str, sizeof(temp_str), cfg_default_value);

	pJson = dave_string_to_json(temp_str, dave_strlen(temp_str));
	if(pJson == NULL)
	{
		BDLOG("invalid access_log_file:%s", temp_str);
		cfg_set_str(cfg_key_name, cfg_default_value);
		return;
	}

	array_len = dave_json_get_array_length(pJson);
	for(array_index=0; array_index<array_len; array_index++)
	{
		log_file = dave_json_array_get_str(pJson, array_index, NULL);
		if(log_file != NULL)
		{
			kv_add_key_ptr(_aliyun_access_kv, log_file, _aliyun_access_kv);
		}
	}

	dave_json_free(pJson);
}

static dave_bool
_aliyun_log_is_access(s8 *log_file)
{
	if(_aliyun_access_kv == NULL)
		return dave_false;

	if(kv_inq_key_ptr(_aliyun_access_kv, log_file) == NULL)
		return dave_false;

	return dave_true;
}

// =====================================================================

dave_bool
aliyun_log_init(void)
{
	s8 project[128];
	s8 logstore[128];
	s8 access_id[128];
	s8 access_key[128];

	dave_memset(project, 0x00, sizeof(project));
	dave_memset(logstore, 0x00, sizeof(logstore));
	dave_memset(access_id, 0x00, sizeof(access_id));
	dave_memset(access_key, 0x00, sizeof(access_key));

	cfg_get_str("AliyunLogProject", project, sizeof(project), "");
	cfg_get_str("AliyunLogStore", logstore, sizeof(logstore), "");
	cfg_get_str("AliyunLogId", access_id, sizeof(access_id), "");
	cfg_get_str("AliyunLogKey", access_key, sizeof(access_key), "");

	if((project[0] == '\0') || (logstore[0] == '\0') || (access_id[0] == '\0') || (access_key[0] == '\0'))
	{
		BDLOG("project:%s logstore:%s access_id:%s access_key:%s has empty param!",
			project, logstore, access_id, access_key);
		return dave_false;
	}

	_aliyun_log_malloc_access();

	return _aliyun_log_init(project, logstore, access_id, access_key);
}

void
aliyun_log_exit(void)
{
	_aliyun_log_exit();

	_aliyun_log_free_access();
}

dave_bool
aliyun_log_string(s8 *log_file, s8 *log_ptr, ub log_len)
{
	if(_aliyun_log_is_access(log_file) == dave_false)
		return dave_true;

	BDTRACE("file:%s log:%s", log_file, log_ptr);

	return _aliyun_log_info(log_file, log_ptr, log_len);
}

dave_bool
aliyun_log_json(s8 *log_file, void *pJson)
{
	void *pLogJson;
	s8 *log_ptr;
	ub log_len;

	if(_aliyun_log_is_access(log_file) == dave_false)
		return dave_true;

	pLogJson = dave_json_get_object(pJson, "log");
	if(pLogJson == NULL)
	{
		BDLOG("log_file:%s can't find log!", log_file);
		return dave_true;
	}

	log_ptr = (s8 *)json_object_to_json_string_length((struct json_object *)(pLogJson), JSON_C_TO_STRING_PLAIN, &log_len);
	if((log_ptr == NULL) || (log_len == 0))
	{
		BDLOG("log_file:%s empty pLogJson! %lx/%ld", log_file, log_ptr, log_len);
		return dave_true;
	}

	BDTRACE("file:%s log:%s", log_file, log_ptr);

	return _aliyun_log_info(log_file, log_ptr, log_len);
}

#endif

