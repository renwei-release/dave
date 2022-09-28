/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
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
#define CFG_ETCD_SERVER_DIR "ETCDServerDir"
#define CFG_ETCD_WATCHER_DIR "ETCDWatcherDir"

#define DEFAULT_ETCD_LIST "http://127.0.0.1:2379"
#define DEFAULT_ETCD_SERVER_DIR "/sync"
#define DEFAULT_ETCD_WATCHER_DIR ""
#define DEFAULT_ETCD_GET_LIMIT 512

#define ETCD_KEY_MAX_SIZE 2048
#define ETCD_VALUE_MAX_SIZE 8192

static s8 _etcd_list[2048] = { "\0" };
static s8 _etcd_dir[128] = { "\0" };
static s8 _etcd_watcher[128] = { "\0" };
static remote_cfg_get_callback _get_callback = NULL;

typedef struct {
	dave_bool put_flag;
	s8 key[ETCD_KEY_MAX_SIZE];
	s8 value[ETCD_VALUE_MAX_SIZE];
} ETCDWatcher;

static s8 *
_sync_server_load_list(void)
{
	return cfg_get_by_default(CFG_ETCD_LIST, _etcd_list, sizeof(_etcd_list), DEFAULT_ETCD_LIST);
}

static s8 *
_sync_server_load_dir(void)
{
	if(_etcd_dir[0] == '\0')
	{
		cfg_get_by_default(CFG_ETCD_SERVER_DIR, _etcd_dir, sizeof(_etcd_dir), DEFAULT_ETCD_SERVER_DIR);
	}

	return _etcd_dir;
}

static s8 *
_sync_server_load_watcher(void)
{
	return cfg_get_by_default(CFG_ETCD_WATCHER_DIR, _etcd_watcher, sizeof(_etcd_watcher), DEFAULT_ETCD_WATCHER_DIR);
}

static s8 *
_sync_server_make_dir(s8 *dir_ptr, ub dir_len, s8 *verno, s8 *globally_identifier, s8 *cfg_name)
{
	s8 product_str[128];

	if(dave_strfindfrist(cfg_name, '/') == NULL)
	{
		dave_snprintf(dir_ptr, dir_len,
			"%s/%s/%s/%s",
			_sync_server_load_dir(),
			dave_verno_product(verno, product_str, sizeof(product_str)),
			globally_identifier,
			cfg_name);
	}
	else
	{
		dave_snprintf(dir_ptr, dir_len, "%s", cfg_name);
	}

	return dir_ptr;
}

static void
_sync_server_process_watcher(MSGBODY *msg)
{
	MsgInnerLoop *pLoop = (MsgInnerLoop *)(msg->msg_body);
	ETCDWatcher *pWatcher = (ETCDWatcher *)(pLoop->ptr);

	SYNCDEBUG("%s %s : %s",
		pWatcher->put_flag==dave_true?"PUT":"DELETE",
		pWatcher->key, pWatcher->value);

	if(_get_callback != NULL)
	{
		_get_callback(pWatcher->put_flag, pWatcher->key, pWatcher->value);
	}

	dave_free(pWatcher);
}

static void
_sync_server_watcher(dave_bool put_flag, s8 *key, s8 *value)
{
	MsgInnerLoop *pLoop = thread_msg(pLoop);

	ETCDWatcher *pWatcher = dave_malloc(sizeof(ETCDWatcher));
	pWatcher->put_flag = put_flag;
	dave_strcpy(pWatcher->key, key, sizeof(pWatcher->key));
	dave_strcpy(pWatcher->value, value, sizeof(pWatcher->value));

	pLoop->ptr = pWatcher;

	name_msg(SYNC_SERVER_THREAD_NAME, MSGID_INNER_LOOP, pLoop);
}

static void
_sync_server_take_watcher_(s8 *key)
{
	void *pArray;
	ub array_len, array_index;
	void *pPutJson;
	s8 json_key[ETCD_KEY_MAX_SIZE], json_value[ETCD_VALUE_MAX_SIZE];

	pArray = dave_etcd_get(key, DEFAULT_ETCD_GET_LIMIT);

	if(pArray != NULL)
	{
		array_len = dave_json_get_array_length(pArray);

		for(array_index=0; array_index<array_len; array_index++)
		{
			pPutJson = dave_json_get_array_idx(pArray, array_index);
			if(pPutJson != NULL)
			{
				dave_json_get_str_v2(pPutJson, "key", json_key, sizeof(json_key));
				dave_json_get_str_v2(pPutJson, "value", json_value, sizeof(json_value));

				_sync_server_watcher(dave_true, json_key, json_value);
			}
		}
	}

	dave_json_free(pArray);
}

static void
_sync_server_take_watcher(void)
{
	s8 *key = _sync_server_load_watcher();

	if(dave_strcmp(key, "") == dave_true)
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
	else
	{
		_sync_server_take_watcher_(key);
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

// =====================================================================

void
remote_etcd_cfg_init(remote_cfg_get_callback get_callback)
{
	_get_callback = get_callback;

	if(_sync_server_etcd_enable() == dave_false)
	{
		return;
	}

	dave_etcd_init(_sync_server_load_list(), _sync_server_load_watcher(), _sync_server_watcher);

	_sync_server_take_watcher();

	reg_msg(MSGID_INNER_LOOP, _sync_server_process_watcher);
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
	s8 dir_key[512];
	dave_bool ret;

	if(_sync_server_etcd_enable() == dave_false)
	{
		return dave_false;
	}

	_sync_server_make_dir(
		dir_key, sizeof(dir_key),
		verno, globally_identifier, cfg_name);

	ret = dave_etcd_set(dir_key, cfg_value, ttl);
	if(ret == dave_false)
	{
		SYNCLOG("set %s:%s failed! ttl:%d", dir_key, cfg_value, ttl);
	}
	else
	{
		SYNCTRACE("set %s:%s success! ttl:%d", dir_key, cfg_value, ttl);
	}

	return ret;
}

dave_bool
remote_etcd_cfg_del(
	s8 *verno, s8 *globally_identifier,
	s8 *cfg_name)
{
	s8 dir_key[512];
	dave_bool ret;

	if(_sync_server_etcd_enable() == dave_false)
	{
		return dave_false;
	}

	_sync_server_make_dir(
		dir_key, sizeof(dir_key),
		verno, globally_identifier, cfg_name);

	ret = dave_etcd_del(dir_key);
	if(ret == dave_false)
	{
		SYNCLOG("del %s failed!", dir_key);
	}
	else
	{
		SYNCTRACE("del %s success!", dir_key);
	}

	return ret;
}

void
remote_etcd_cfg_get(remote_cfg_get_callback get_callback)
{
	_get_callback = get_callback;
}

#endif
