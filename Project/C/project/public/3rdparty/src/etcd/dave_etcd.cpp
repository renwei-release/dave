/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(ETCD_3RDPARTY)
#include <iostream>
#include <cstring>
#include <string>
#include "etcd/Client.hpp"
#include "etcd/SyncClient.hpp"
#include "etcd/Watcher.hpp"
#include "etcd/KeepAlive.hpp"
#include "etcd/Value.hpp"
#include "dave_base.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "party_log.h"

#define ETCD_DEFAULT_KEEPALIVE 60

static s8 _etcd_client_url[256];
static s8 _etcd_watcher_dir[256];
static etcd_watcher_fun _watcher_fun = NULL;

static etcd::SyncClient *_etcd_client = NULL;
static etcd::Watcher *_etcd_watcher = NULL;
static etcd::KeepAlive *_etcd_keepalive = NULL;
static int64_t _etcd_leaseid = 0;

static void
_etcd_watcher_response(etcd::Response const & resp)
{
	if(resp.error_code())
	{
		PARTYLOG("error_code:%d/%s",
			resp.error_code(),
			resp.error_message().c_str());
		return;
	}

	for(auto const &ev: resp.events())
	{
		etcd::Event::EventType event_type;
		const char *key, *value;

		event_type = ev.event_type();
		key = ev.kv().key().c_str();
		value = ev.kv().as_string().c_str();

		if(event_type == etcd::Event::EventType::PUT)
		{
			_watcher_fun(dave_true, (s8 *)key, (s8 *)value);
		}
		else if(event_type == etcd::Event::EventType::DELETE_)
		{
			_watcher_fun(dave_false, (s8 *)key, (s8 *)value);
		}
		else
		{
			PARTYABNOR("%d %s:%s", event_type, key, value);
		}
	}
}

static void
_etcd_init(void)
{
	_etcd_client = new etcd::SyncClient(_etcd_client_url);
	_etcd_watcher = new etcd::Watcher(_etcd_client_url, _etcd_watcher_dir, _etcd_watcher_response, true);

	etcd::Response lease = _etcd_client->leasegrant((int)ETCD_DEFAULT_KEEPALIVE);
	_etcd_leaseid = lease.value().lease();

	_etcd_keepalive = new etcd::KeepAlive(_etcd_client_url, (int)ETCD_DEFAULT_KEEPALIVE, _etcd_leaseid);
}

static void
_etcd_exit(void)
{
	if(_etcd_client != NULL)
	{
		delete _etcd_client;
		_etcd_client = NULL;
	}
	if(_etcd_watcher != NULL)
	{
		delete _etcd_watcher;
		_etcd_watcher = NULL;
	}
	if(_etcd_keepalive != NULL)
	{
		delete _etcd_keepalive;
		_etcd_keepalive = NULL;
	}
}

static void
_etcd_reset(void)
{
	_etcd_exit();

	dave_os_sleep(1000);

	_etcd_init();
}

static dave_bool
_etcd_set(s8 *key, s8 *value, sb ttl)
{
	if(ttl <= 0)
	{
		etcd::Response resp = _etcd_client->set(key, value, 0);

		if(0 != resp.error_code())
		{
			PARTYLOG("set key:%s value:%s failed:%d/%s",
				key, value,
				resp.error_code(),
				resp.error_message().c_str());
			return dave_false;
		}
	}
	else
	{
		etcd::Response resp = _etcd_client->set(key, value, _etcd_leaseid);

		if(0 != resp.error_code())
		{
			PARTYLOG("set key:%s value:%s failed:%d/%s",
				key, value,
				resp.error_code(),
				resp.error_message().c_str());
			return dave_false;
		}
	}

	return dave_true;
}

static void *
_etcd_get(s8 *key, ub limit)
{
	void *pArray;
	int index, size;

	etcd::Response resp = _etcd_client->ls(key, limit);

	if(resp.error_code())
	{
		PARTYLOG("url:%s key:%s error_code:%d/%s",
			_etcd_client_url, key,
			resp.error_code(),
			resp.error_message().c_str());
		return NULL;
	}

	pArray = dave_json_array_malloc();

	size = (int)(resp.keys().size());

	PARTYDEBUG("url:%s key:%s size:%d", _etcd_client_url, key, size);

	for(index=0; index<size; index++)
	{
		PARTYDEBUG("key:%s value:%s", resp.keys()[index].c_str(), resp.values()[index].as_string().c_str());
	
		void *pPutJson = dave_json_malloc();
		dave_json_add_str(pPutJson, (char *)"key", (s8 *)(resp.keys()[index].c_str()));
		dave_json_add_str(pPutJson, (char *)"value", (s8 *)(resp.values()[index].as_string().c_str()));
		dave_json_array_add_object(pArray, pPutJson);
	}

	return pArray;
}

static dave_bool
_etcd_del(s8 *key)
{
	etcd::Response resp = _etcd_client->rm(key);

	if(0 != resp.error_code())
	{
		PARTYLOG("del key:%s failed:%d/%s",
			key,
			resp.error_code(),
			resp.error_message().c_str());
		return dave_false;
	}

	return dave_true;
}

// =====================================================================

extern "C" void
dave_etcd_init(s8 *url, s8 *watcher_dir, etcd_watcher_fun watcher_fun)
{
	dave_strcpy(_etcd_client_url, url, sizeof(_etcd_client_url));
	dave_strcpy(_etcd_watcher_dir, watcher_dir, sizeof(_etcd_watcher_dir));
	_watcher_fun = watcher_fun;

	_etcd_init();
}

extern "C" void
dave_etcd_exit(void)
{
	_etcd_exit();
}

extern "C" dave_bool
dave_etcd_set(s8 *key, s8 *value, sb ttl)
{
	dave_bool ret;

	ret = _etcd_set(key, value, ttl);
	if(ret == dave_false)
	{
		PARTYLOG("find error! reset etcd client!");

		_etcd_reset();

		ret = _etcd_set(key, value, ttl);
	}

	return ret;
}

extern "C" void *
dave_etcd_get(s8 *key, ub limit)
{
	void *pArray;

	pArray = _etcd_get(key, limit);
	if(pArray == NULL)
	{
		PARTYLOG("find error! reset etcd client!");

		_etcd_reset();

		pArray = _etcd_get(key, limit);			
	}

	return pArray;
}

extern "C" dave_bool
dave_etcd_del(s8 *key)
{
	dave_bool ret;

	ret = _etcd_del(key);
	if(ret == dave_false)
	{
		PARTYLOG("find error! reset etcd client!");

		_etcd_reset();

		ret = _etcd_del(key);
	}

	return ret;
}

#endif

