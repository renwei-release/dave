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
#include "sync_server_remote_cfg.h"
#include "sync_log.h"

#define CFG_ETCD_LIST "ETCDServerList"
#define CFG_ETCD_SERVER_DIR "ETCDServerDir"
#define CFG_ETCD_WATCHER_DIR "ETCDWatcherDir"

#define DEFAULT_ETCD_LIST "http://127.0.0.1:2379"
#define DEFAULT_ETCD_SERVER_DIR "/sync"
#define DEFAULT_ETCD_WATCHER_DIR "/"
#define DEFAULT_ETCD_GET_LIMIT 20480

static s8 _etcd_list[2048] = { "\0" };
static s8 _etcd_dir[128] = { "\0" };
static s8 _etcd_watcher[128] = { "\0" };

typedef struct {
	dave_bool put_flag;
	s8 key[2048];
	s8 value[4096];
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

	dave_snprintf(dir_ptr, dir_len,
		"%s/%s/%s/%s",
		_sync_server_load_dir(),
		dave_verno_product(verno, product_str, sizeof(product_str)),
		globally_identifier,
		cfg_name);

	return dir_ptr;
}

static void
_sync_server_process_watcher(MSGBODY *msg)
{
	MsgInnerLoop *pLoop = (MsgInnerLoop *)(msg->msg_body);
	ETCDWatcher *pWatcher = (ETCDWatcher *)(pLoop->ptr);

	SYNCTRACE("%s key:%s value:%s",
		pWatcher->put_flag==dave_true?"PUT":"DELETE",
		pWatcher->key, pWatcher->value);

	if(pWatcher->put_flag == dave_true)
	{
		base_cfg_remote_internal_add(pWatcher->key, pWatcher->value);
	}
	else
	{
		base_cfg_remote_internal_del(pWatcher->key);
	}

	sync_server_remote_cfg_tell_config(pWatcher->put_flag, pWatcher->key, pWatcher->value);

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
	void *pArray = dave_etcd_get(key, DEFAULT_ETCD_GET_LIMIT);
	ub array_len, array_index;
	void *pPutJson;
	s8 json_key[1024], json_value[4096];

	if(pArray != NULL)
	{
		array_len = dave_json_get_array_length(pArray);

		for(array_index=0; array_index<array_len; array_index++)
		{
			pPutJson = dave_json_get_array_idx(pArray, array_index);
			if(pPutJson != NULL)
			{
				dave_json_get_str_v2(pPutJson, (char *)"key", json_key, sizeof(json_key));
				dave_json_get_str_v2(pPutJson, (char *)"value", json_value, sizeof(json_value));

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

	if(dave_strcmp(key, "/") == dave_true)
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

// =====================================================================

void
remote_etcd_cfg_init(void)
{
	dave_etcd_init(_sync_server_load_list(), _sync_server_load_watcher(), _sync_server_watcher);

	_sync_server_take_watcher();

	reg_msg(MSGID_INNER_LOOP, _sync_server_process_watcher);
}

void
remote_etcd_cfg_exit(void)
{
	unreg_msg(MSGID_INNER_LOOP);

	dave_etcd_exit();
}

dave_bool
remote_etcd_cfg_set(s8 *verno, s8 *globally_identifier, s8 *cfg_name, s8 *cfg_value)
{
	s8 dir_key[512];
	dave_bool ret;

	_sync_server_make_dir(
		dir_key, sizeof(dir_key),
		verno, globally_identifier, cfg_name);

	ret = dave_etcd_set(dir_key, cfg_value);
	if(ret == dave_false)
	{
		SYNCLOG("set %s:%s failed!", dir_key, cfg_value);
	}
	else
	{
		SYNCTRACE("set %s:%s success!", dir_key, cfg_value);
	}

	return ret;
}

#endif

