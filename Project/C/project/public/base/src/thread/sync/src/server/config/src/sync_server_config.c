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
#include "sync_server_config.h"
#include "sync_server_app_tx.h"
#include "sync_log.h"

#define CFG_ETCD_LIST "ETCDServerList"
#define CFG_ETCD_SERVER_DIR "ETCDServerDir"
#define CFG_ETCD_WATCHER_DIR "ETCDWatcherDir"

#define DEFAULT_ETCD_LIST "http://127.0.0.1:2379"
#define DEFAULT_ETCD_SERVER_DIR "/sync"
#define DEFAULT_ETCD_WATCHER_DIR "/sync"

static s8 _etcd_dir[128] = { "\0" };
static s8 _etcd_watcher[128] = { "\0" };

typedef struct {
	dave_bool put_flag;
	s8 key[2048];
	s8 value[4096];
} ETCDWatcher;

static s8 *
_sync_server_load_list(s8 *list_ptr, ub list_len)
{
	if(cfg_get(CFG_ETCD_LIST, list_ptr, list_len) == 0)
	{
		dave_strcpy(list_ptr, DEFAULT_ETCD_LIST, list_len);
		cfg_set(CFG_ETCD_LIST, list_ptr, dave_strlen(list_ptr));
	}

	return list_ptr;
}

static s8 *
_sync_server_load_dir(void)
{
	s8 *dir_ptr = _etcd_dir;
	ub dir_len = sizeof(_etcd_dir);

	if(cfg_get(CFG_ETCD_SERVER_DIR, dir_ptr, dir_len) == 0)
	{
		dave_strcpy(dir_ptr, DEFAULT_ETCD_SERVER_DIR, dir_len);
		cfg_set(CFG_ETCD_SERVER_DIR, dir_ptr, dave_strlen(dir_ptr));
	}

	return dir_ptr;
}

static s8 *
_sync_server_load_watcher(void)
{
	s8 *watcher_ptr = _etcd_watcher;
	ub watcher_len = sizeof(_etcd_watcher);

	if(cfg_get(CFG_ETCD_WATCHER_DIR, watcher_ptr, watcher_len) == 0)
	{
		dave_strcpy(watcher_ptr, DEFAULT_ETCD_WATCHER_DIR, watcher_len);
		cfg_set(CFG_ETCD_WATCHER_DIR, watcher_ptr, dave_strlen(watcher_ptr));
	}

	return watcher_ptr;
}

static s8 *
_sync_server_load_set_dir(SyncClient *pClient)
{
	if(pClient->remote_config_dir[0] == '\0')
	{
		s8 product_str[128];

		dave_snprintf(pClient->remote_config_dir, sizeof(pClient->remote_config_dir),
			"%s/%s/%s",
			_etcd_dir,
			dave_verno_product(pClient->verno, product_str, sizeof(product_str)),
			pClient->globally_identifier);
	}

	return pClient->remote_config_dir;
}

static void
_sync_server_the_config_tell_all_client(dave_bool put_flag, s8 *key, s8 *value)
{
	CFGRemoteUpdate update;

	update.put_flag = put_flag;
	dave_strcpy(update.cfg_name, key, sizeof(update.cfg_name));
	dave_strcpy(update.cfg_value, value, sizeof(update.cfg_value));

	sync_server_app_tx_all_client(MSGID_CFG_REMOTE_UPDATE, sizeof(CFGRemoteUpdate), &update);
}

static void
_sync_server_the_client_tell_all_config(SyncClient *pClient)
{
	CFGRemoteUpdate update;
	ub index;

	for(index=0; index<102400; index++)
	{
		update.put_flag = dave_true;
		if(base_cfg_remote_index(
			index,
			update.cfg_name, sizeof(update.cfg_name),
			update.cfg_value, sizeof(update.cfg_value)) == 0)
		{
			break;
		}

		sync_server_app_tx_client(pClient, MSGID_CFG_REMOTE_UPDATE, sizeof(CFGRemoteUpdate), &update);
	}
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

	_sync_server_the_config_tell_all_client(pWatcher->put_flag, pWatcher->key, pWatcher->value);

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
_sync_server_take_watcher(void)
{
	void *pArray = dave_etcd_get(_sync_server_load_watcher());
	ub array_len, array_index;
	void *pPutJson;
	s8 key[1024], value[1024];

	if(pArray != NULL)
	{
		array_len = dave_json_get_array_length(pArray);

		for(array_index=0; array_index<array_len; array_index++)
		{
			pPutJson = dave_json_get_array_idx(pArray, array_index);
			if(pPutJson != NULL)
			{
				dave_json_get_str_v2(pPutJson, (char *)"key", key, sizeof(key));
				dave_json_get_str_v2(pPutJson, (char *)"value", value, sizeof(value));

				_sync_server_watcher(dave_true, key, value);
			}
		}
	}

	dave_json_free(pArray);
}

// =====================================================================

void
sync_server_config_init(void)
{
	s8 etcd_list[2049];

	_sync_server_load_list(etcd_list, sizeof(etcd_list));
	_sync_server_load_dir();
	_sync_server_load_watcher();

	dave_etcd_init(etcd_list, _sync_server_load_watcher(), _sync_server_watcher);

	_sync_server_take_watcher();

	reg_msg(MSGID_INNER_LOOP, _sync_server_process_watcher);
}

void
sync_server_config_exit(void)
{
	unreg_msg(MSGID_INNER_LOOP);

	dave_etcd_exit();
}

dave_bool
sync_server_config_set(SyncClient *pClient, CFGRemoteUpdate *pUpdate)
{
	s8 dir_key[1024];
	dave_bool ret;

	dave_snprintf(dir_key, sizeof(dir_key),
		"%s/%s",
		_sync_server_load_set_dir(pClient), pUpdate->cfg_name);

	ret = dave_etcd_set(dir_key, pUpdate->cfg_value);
	if(ret == dave_false)
	{
		SYNCLOG("set %s:%s failed!", dir_key, pUpdate->cfg_value);
	}
	else
	{
		SYNCTRACE("set %s:%s success!", dir_key, pUpdate->cfg_value);
	}

	return ret;
}

void
sync_server_config_tell_client(SyncClient *pClient)
{
	_sync_server_the_client_tell_all_config(pClient);
}

#endif

