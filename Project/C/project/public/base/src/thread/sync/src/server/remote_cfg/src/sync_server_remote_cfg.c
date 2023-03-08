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
#include "remote_etcd_cfg.h"
#include "sync_server_app_tx.h"
#include "sync_log.h"

#define REMOTE_BASE_TIMER 3

typedef struct {
	SyncClient *pClient;
	s32 socket;
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];

	CFGRemoteSyncUpdate update;
} RemoteCfgReflash;

static void *_remote_cfg_reflash_kv = NULL;

static void
_sync_server_the_config_tell_all_client(dave_bool put_flag, s8 *key, s8 *value)
{
	CFGRemoteSyncUpdate update;
	ub client_index;

	dave_memset(&update, 0x00, sizeof(update));

	update.put_flag = put_flag;
	update.cfg_mbuf_name = t_a2b_str_to_mbuf(key, 0);
	update.cfg_mbuf_value = t_a2b_str_to_mbuf(value, 0);
	update.ttl = 0;

	/*
	 * Prevent MBUF from being released by this call:
	 * sync_server_app_tx_all_client -> sync_server_tx_run_internal_msg_v2_req ->
	 * t_rpc_zip
	 */
	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		dave_mref(update.cfg_mbuf_name);
		dave_mref(update.cfg_mbuf_value);
	}

	sync_server_app_tx_all_client(MSGID_CFG_REMOTE_SYNC_UPDATE, sizeof(CFGRemoteSyncUpdate), &update);

	dave_mclean(update.cfg_mbuf_name);
	dave_mclean(update.cfg_mbuf_value);
}

static void
_sync_server_the_client_tell_all_config(SyncClient *pClient)
{
	CFGRemoteSyncUpdate update;
	s8 *cfg_name_ptr;
	ub cfg_name_len = 4096;
	s8 *cfg_value_ptr;
	ub cfg_value_len = 1024 * 1024;
	ub index;
	dave_bool ret;

	cfg_name_ptr = dave_malloc(cfg_name_len);
	cfg_value_ptr = dave_malloc(cfg_value_len);

	for(index=0; index<9999999; index++)
	{
		dave_memset(&update, 0x00, sizeof(update));

		update.put_flag = dave_true;
		if(rcfg_index(
			index,
			cfg_name_ptr, cfg_name_len,
			cfg_value_ptr, cfg_value_len) < 0)
		{
			break;
		}
		update.cfg_mbuf_name = t_a2b_str_to_mbuf(cfg_name_ptr, 0);
		update.cfg_mbuf_value = t_a2b_str_to_mbuf(cfg_value_ptr, 0);
		update.ttl = 0;

		if((ms8(update.cfg_mbuf_name))[0] != '\0')
		{
			ret = sync_server_app_tx_client(pClient, MSGID_CFG_REMOTE_SYNC_UPDATE, sizeof(CFGRemoteSyncUpdate), &update);
		}
		else
		{
			ret = dave_false;
		}

		if(ret == dave_false)
		{
			dave_mfree(update.cfg_mbuf_name);
			dave_mfree(update.cfg_mbuf_value);
		}
	}

	dave_free(cfg_name_ptr);
	dave_free(cfg_value_ptr);
}

static void
_sync_server_cfg_update(dave_bool put_flag, s8 *key, s8 *value)
{
	SYNCTRACE("%s %s : %s", put_flag==dave_true?"PUT":"DELETE", key, value);

	if(put_flag == dave_true)
	{
		base_cfg_remote_internal_add(key, value);
	}
	else
	{
		base_cfg_remote_internal_del(key);
	}

	_sync_server_the_config_tell_all_client(put_flag, key, value);
}

static s8 *
_sync_server_cfg_kv_key(s8 *key_ptr, ub key_len, s8 *globally_identifier, s8 *cfg_name)
{
	dave_snprintf(key_ptr, key_len, "%s-%s", globally_identifier, cfg_name);
	return key_ptr;
}

static RemoteCfgReflash *
_sync_server_cfg_reflash_malloc(SyncClient *pClient, CFGRemoteSyncUpdate *pUpdate)
{
	RemoteCfgReflash *pReflash;

	pReflash = dave_malloc(sizeof(RemoteCfgReflash));

	pReflash->pClient = pClient;
	pReflash->socket = pClient->client_socket;
	dave_strcpy(pReflash->verno, pClient->verno, sizeof(pReflash->verno));
	dave_strcpy(pReflash->globally_identifier, pClient->globally_identifier, sizeof(pReflash->globally_identifier));

	pReflash->update.put_flag = pUpdate->put_flag;
	pReflash->update.cfg_mbuf_name = dave_mclone(pUpdate->cfg_mbuf_name);
	pReflash->update.cfg_mbuf_value = dave_mclone(pUpdate->cfg_mbuf_value);
	pReflash->update.ttl = pUpdate->ttl;

	return pReflash;
}

static void
_sync_server_cfg_reflash_free(RemoteCfgReflash *pReflash)
{
	dave_mfree(pReflash->update.cfg_mbuf_name);
	dave_mfree(pReflash->update.cfg_mbuf_value);

	dave_free(pReflash);
}

static dave_bool
_sync_server_cfg_kv_del(s8 *globally_identifier, s8 *cfg_name)
{
	RemoteCfgReflash *pReflash;
	s8 key[128];

	_sync_server_cfg_kv_key(key, sizeof(key), globally_identifier, cfg_name);

	pReflash = kv_del_key_ptr(_remote_cfg_reflash_kv, key);

	if(pReflash != NULL)
	{
		SYNCTRACE("%s %s ttl:%d", globally_identifier, cfg_name, pReflash->update.ttl);

		remote_etcd_cfg_del(
			pReflash->verno, pReflash->globally_identifier,
			ms8(pReflash->update.cfg_mbuf_name));

		_sync_server_cfg_reflash_free(pReflash);

		return dave_true;
	}

	return dave_false;
}

static dave_bool
_sync_server_cfg_kv_add(SyncClient *pClient, CFGRemoteSyncUpdate *pUpdate)
{
	RemoteCfgReflash *pReflash;
	s8 key[128];
	dave_bool ret;

	if((pUpdate->put_flag == dave_false) || (pUpdate->ttl <= 0))
	{
		return dave_false;
	}

	_sync_server_cfg_kv_key(key, sizeof(key), pClient->globally_identifier, ms8(pUpdate->cfg_mbuf_name));

	pReflash = kv_inq_key_ptr(_remote_cfg_reflash_kv, key);
	if(pReflash != NULL)
	{
		return dave_true;
	}

	pReflash = _sync_server_cfg_reflash_malloc(pClient, pUpdate);

	kv_add_key_ptr(_remote_cfg_reflash_kv, key, pReflash);

	ret = remote_etcd_cfg_set(
		pReflash->verno, pReflash->globally_identifier,
		ms8(pReflash->update.cfg_mbuf_name), ms8(pReflash->update.cfg_mbuf_value),
		pReflash->update.ttl);

	SYNCTRACE("%s %s ttl:%d", pClient->globally_identifier, ms8(pUpdate->cfg_mbuf_name), pUpdate->ttl);

	if(ret == dave_false)
	{
		_sync_server_cfg_kv_del(pClient->globally_identifier, ms8(pUpdate->cfg_mbuf_name));
	}

	return ret;
}

static void
_sync_server_cfg_kv_timer_out(void *ramkv, s8 *key)
{
	RemoteCfgReflash *pReflash = kv_inq_key_ptr(ramkv, key);

	/*
	 * Whether the client detected by the three REMOTE_BASE_TIMER
	 * interval is not online, 
	 * if not online, delete parameter configuration information.
	 */
	if(pReflash->pClient->client_socket != pReflash->socket)
	{
		_sync_server_cfg_kv_del(pReflash->globally_identifier, ms8(pReflash->update.cfg_mbuf_name));
	}
}

static RetCode
_sync_server_cfg_kv_recycle(void *ramkv, s8 *key)
{
	RemoteCfgReflash *pReflash;

	pReflash = kv_del_key_ptr(ramkv, key);
	if(pReflash == NULL)
		return RetCode_empty_data;

	_sync_server_cfg_reflash_free(pReflash);

	return RetCode_OK;
}

// =====================================================================

void
sync_server_remote_cfg_init(void)
{
	_remote_cfg_reflash_kv = kv_malloc("ssrcr", REMOTE_BASE_TIMER, _sync_server_cfg_kv_timer_out);

	remote_etcd_cfg_init(_sync_server_cfg_update);

	remote_etcd_cfg_get(_sync_server_cfg_update);
}

void
sync_server_remote_cfg_exit(void)
{
	kv_free(_remote_cfg_reflash_kv, _sync_server_cfg_kv_recycle); _remote_cfg_reflash_kv = NULL;

	remote_etcd_cfg_exit();
}

dave_bool
sync_server_remote_cfg_set(SyncClient *pClient, CFGRemoteSyncUpdate *pUpdate)
{
	dave_bool ret;

	SYNCTRACE("%s %s ttl:%d", pClient->globally_identifier, ms8(pUpdate->cfg_mbuf_name), pUpdate->ttl);

	if(pUpdate->ttl > 0)
	{
		ret = _sync_server_cfg_kv_add(pClient, pUpdate);
	}
	else
	{
		ret = remote_etcd_cfg_set(
			pClient->verno, pClient->globally_identifier,
			ms8(pUpdate->cfg_mbuf_name), ms8(pUpdate->cfg_mbuf_value),
			0);
	}

	return ret;
}

dave_bool
sync_server_remote_cfg_del(SyncClient *pClient, CFGRemoteSyncUpdate *pUpdate)
{
	SYNCTRACE("%s %s ttl:%d", pClient->globally_identifier, ms8(pUpdate->cfg_mbuf_name), pUpdate->ttl);

	if(_sync_server_cfg_kv_del(pClient->globally_identifier, ms8(pUpdate->cfg_mbuf_name)) == dave_false)
	{
		return remote_etcd_cfg_del(
			pClient->verno, pClient->globally_identifier,
			ms8(pUpdate->cfg_mbuf_name));
	}

	return dave_true;
}

void
sync_server_remote_cfg_tell_client(SyncClient *pClient)
{
	_sync_server_the_client_tell_all_config(pClient);
}

#endif

