/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
/*
 * 20230224 --- chenxiaomin cxm2048@163.com
 * Fix the etcd get error,
 * add the _sync_server_traverse_etcd_prefix function.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "remote_etcd_cfg.h"
#include "sync_log.h"

#define CFG_ETCD_LIST "ETCDServerList"
#define CFG_ETCD_WATCHER_DIR "ETCDWatcherDir"
#define CFG_ETCD_GET_DIR "ETCDGetDir"

#define DEFAULT_ETCD_LIST "http://127.0.0.1:2379"
#define DEFAULT_ETCD_SERVER_DIR "/sync"
#define DEFAULT_ETCD_WATCHER_DIR "/"
#define DEFAULT_ETCD_GET_DIR "/cnf;"
#define DEFAULT_ETCD_GET_LIMIT 0	// if zero, get all etcd config.

static s8 _etcd_list[2048] = { "\0" };
static s8 _etcd_watcher[128] = { "\0" };
static s8 _etcd_get[4192] = { "\0" };
static remote_cfg_get_callback _get_callback = NULL;

typedef struct {
	dave_bool put_flag;
	MBUF *key;
	MBUF *value;
} ETCDWatcher;

static s8 *
_sync_server_load_list(void)
{
	return cfg_get_by_default(CFG_ETCD_LIST, _etcd_list, sizeof(_etcd_list), DEFAULT_ETCD_LIST);
}

static void
_sync_server_clean_list(void)
{
	cfg_set_str(CFG_ETCD_LIST, "");
}

static void
_sync_server_set_list(s8 *list)
{
	cfg_set_str(CFG_ETCD_LIST, list);
}

static s8 *
_sync_server_load_watcher(void)
{
	return cfg_get_by_default(CFG_ETCD_WATCHER_DIR, _etcd_watcher, sizeof(_etcd_watcher), DEFAULT_ETCD_WATCHER_DIR);
}

static s8 *
_sync_server_load_get(void)
{
	return cfg_get_by_default(CFG_ETCD_GET_DIR, _etcd_get, sizeof(_etcd_get), DEFAULT_ETCD_GET_DIR);
}

static void
_sync_server_process_watcher(MSGBODY *msg)
{
	MsgInnerLoop *pLoop = (MsgInnerLoop *)(msg->msg_body);
	ETCDWatcher *pWatcher = (ETCDWatcher *)(pLoop->ptr);

	SYNCTRACE("%s %s : %s",
		pWatcher->put_flag==dave_true?"PUT":"DELETE",
		base_mptr(pWatcher->key), base_mptr(pWatcher->value));

	if(_get_callback != NULL)
	{
		_get_callback(pWatcher->put_flag, base_mptr(pWatcher->key), base_mptr(pWatcher->value));
	}

	dave_mfree(pWatcher->key);
	dave_mfree(pWatcher->value);

	dave_free(pWatcher);
}

static void
_sync_server_watcher(dave_bool put_flag, s8 *key, s8 *value)
{
	MsgInnerLoop *pLoop = thread_msg(pLoop);

	ETCDWatcher *pWatcher = dave_malloc(sizeof(ETCDWatcher));

	pWatcher->put_flag = put_flag;
	pWatcher->key = t_a2b_str_to_mbuf(key, -1);
	pWatcher->value = t_a2b_str_to_mbuf(value, -1);

	pLoop->ptr = pWatcher;

	name_msg(SYNC_SERVER_THREAD_NAME, MSGID_INNER_LOOP, pLoop);
}

static void
_sync_server_take_watcher_(s8 *key)
{
	void *pArray;
	ub array_len, array_index;
	void *pPutJson;
	s8 *key_key = "key", *value_key = "value";
	ub key_len, value_len;
	s8 *json_key, *json_value;

	pArray = dave_etcd_get(key, DEFAULT_ETCD_GET_LIMIT);

	if(pArray != NULL)
	{
		array_len = dave_json_get_array_length(pArray);

		if((array_len >= DEFAULT_ETCD_GET_LIMIT) && (DEFAULT_ETCD_GET_LIMIT != 0))
		{
			SYNCLOG("Note that the obtained value beyond the maximum(%ld/%ld) qualifier, you may not be able to get a complete value.",
				array_len, DEFAULT_ETCD_GET_LIMIT);
		}

		for(array_index=0; array_index<array_len; array_index++)
		{
			pPutJson = dave_json_get_array_idx(pArray, array_index);
			if(pPutJson != NULL)
			{
				key_len = dave_json_get_str_length(pPutJson, key_key) + 1;
				value_len = dave_json_get_str_length(pPutJson, value_key) + 1;

				json_key = dave_malloc(key_len);
				json_value = dave_malloc(value_len);

				dave_json_get_str_v2(pPutJson, key_key, json_key, key_len);
				dave_json_get_str_v2(pPutJson, value_key, json_value, value_len);

				_sync_server_watcher(dave_true, json_key, json_value);

				dave_free(json_key);
				dave_free(json_value);
			}
		}
	}

	dave_json_free(pArray);
}

static void 
_sync_server_traverse_etcd_prefix(s8 *key_str)
{
	ub i;
	ub index = 0;
	s8 *key;
	ub len = dave_strlen(key_str);
	
	for(i = 0; i < len; i++)
	{
		if(key_str[i] == ';')
		{
			key = dave_malloc(i - index + 1);

			dave_strcpy(key, &key_str[index], i - index + 1);
			index = i + 1;
			_sync_server_take_watcher_(key);

			dave_free(key);
		}
	}

	if((key_str[len - 1] != ';') && (index < len))
	{
		key = dave_malloc(len - index + 1);

		dave_strcpy(key, &key_str[index], len - index + 1);
		_sync_server_take_watcher_(key);

		dave_free(key);
	}
}

static void
_sync_server_get_all_key(void)
{
	s8 key_loop;
	s8 key_str[2];
	
	/*
	 * get all config
	 */
	for(key_loop='0'; key_loop<='9'; key_loop++)
	{
		key_str[0] = key_loop;
		key_str[1] = '\0';
		_sync_server_take_watcher_(key_str);
	}

	for(key_loop='a'; key_loop<='z'; key_loop++)
	{
		key_str[0] = key_loop;
		key_str[1] = '\0';
		_sync_server_take_watcher_(key_str);
	}

	for(key_loop='A'; key_loop<='Z'; key_loop++)
	{
		key_str[0] = key_loop;
		key_str[1] = '\0';
		_sync_server_take_watcher_(key_str);
	}

	key_str[0] = '/';
	key_str[1] = '\0';
	_sync_server_take_watcher_(key_str);
}

static void
_sync_server_take_get(void)
{
	s8 *key = _sync_server_load_get();

	if(dave_strcmp(key, "") == dave_true)
	{
		_sync_server_get_all_key();
	}
	else
	{
		_sync_server_traverse_etcd_prefix(key);
	}
}

static dave_bool
_sync_server_etcd_enable(void)
{
	_sync_server_load_list();

	if(dave_strlen(_etcd_list) == 0)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

static void
_sync_server_prevent_restart_init(void)
{
	s8 *list;

	list = _sync_server_load_list();

	/*
	 * 先清空配置，如果在dave_etcd_init中重启，
	 * 重启后，_sync_server_etcd_enable就判断失效的配置，
	 * 不至于引起无限重启。
	 */
	_sync_server_clean_list();

	dave_etcd_init(list, _sync_server_load_watcher(), _sync_server_watcher);

	_sync_server_set_list(list);
}

// =====================================================================

void
remote_etcd_cfg_init(remote_cfg_get_callback get_callback)
{
	_get_callback = get_callback;

	if(_sync_server_etcd_enable() == dave_false)
	{
		return;
	}

	reg_msg(MSGID_INNER_LOOP, _sync_server_process_watcher);

	_sync_server_prevent_restart_init();

	_sync_server_take_get();
}

void
remote_etcd_cfg_exit(void)
{
	if(_sync_server_etcd_enable() == dave_false)
	{
		return;
	}

	unreg_msg(MSGID_INNER_LOOP);

	dave_etcd_exit();
}

dave_bool
remote_etcd_cfg_set(
	s8 *verno, s8 *globally_identifier,
	s8 *cfg_name, s8 *cfg_value,
	sb ttl)
{
	dave_bool ret;

	if(_sync_server_etcd_enable() == dave_false)
	{
		return dave_false;
	}

	ret = dave_etcd_set(cfg_name, cfg_value, ttl);
	if(ret == dave_false)
	{
		SYNCLOG("set %s:%s failed! ttl:%d", cfg_name, cfg_value, ttl);
	}
	else
	{
		SYNCTRACE("set %s:%s success! ttl:%d", cfg_name, cfg_value, ttl);
	}

	return ret;
}

dave_bool
remote_etcd_cfg_del(
	s8 *verno, s8 *globally_identifier,
	s8 *cfg_name)
{
	dave_bool ret;

	if(_sync_server_etcd_enable() == dave_false)
	{
		return dave_false;
	}

	ret = dave_etcd_del(cfg_name);
	if(ret == dave_false)
	{
		SYNCLOG("del %s failed!", cfg_name);
	}
	else
	{
		SYNCTRACE("del %s success!", cfg_name);
	}

	return ret;
}

void
remote_etcd_cfg_get(remote_cfg_get_callback get_callback)
{
	_get_callback = get_callback;
}

#endif

