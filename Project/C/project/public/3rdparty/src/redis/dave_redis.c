/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(REDIS_3RDPARTY)
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <dlfcn.h>
#include "hiredis.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "party_log.h"

#define REDIS_CONNECT_TIMEOUT_TIME 3

static void *
_redis_reply_string_to_json(redisReply *redis_reply)
{
	void *pJson = dave_json_malloc();

	dave_json_add_str_len(pJson, "STRING", redis_reply->str, redis_reply->len);

	return pJson;
}

static void *
_redis_reply_integer_to_json(redisReply *redis_reply)
{
	void *pJson = dave_json_malloc();

	dave_json_add_sb(pJson, "INTEGER", redis_reply->integer);

	return pJson;
}

static void *
_redis_reply_nil_to_json(redisReply *redis_reply)
{
	void *pJson = dave_json_malloc();

	dave_json_add_str(pJson, "NIL", "NIL");

	return pJson;
}

static void *
_redis_reply_to_json(redisReply *redis_reply)
{
	void *pJson = NULL;

	switch(redis_reply->type)
	{
		case REDIS_REPLY_STRING:
				pJson = _redis_reply_string_to_json(redis_reply);
			break;
		case REDIS_REPLY_INTEGER:
				pJson = _redis_reply_integer_to_json(redis_reply);
			break;
		case REDIS_REPLY_NIL:
				pJson = _redis_reply_nil_to_json(redis_reply);
			break;
		default:
				PARTYABNOR("unsupport type:%d", redis_reply->type);
			break;
	}

	return pJson;
}

static void
_redis_free_reply(void *reply)
{
	if(reply == NULL)
	{
		PARTYABNOR("redis reply is null!");
		return;
	}
	
	freeReplyObject(reply);
	
	return;
}

// =====================================================================

void *
dave_redis_connect(s8 *ip, ub port)
{
	redisContext* context = NULL;
	struct timeval tv;

	tv.tv_sec = REDIS_CONNECT_TIMEOUT_TIME;
	tv.tv_usec = 0;
	context = redisConnectWithTimeout((const char *)ip, (int)port, tv);

	if(context == NULL || context->err) 
	{
	    if(context)
		{
	        PARTYABNOR("Error: %s", context->errstr);
	    } 
		else
		{
	        PARTYABNOR("Can't allocate redis context");
		}

		return NULL;
	}

	return (void*)context; 
}

void
dave_redis_disconnect(void *context)
{
	redisContext *redis_context = NULL;

	if(context == NULL)
	{
		PARTYABNOR("redis context is null!");
		return;
	}
	else
	{
		redis_context = (redisContext *)context;
	}

	if(redis_context != NULL)
	{
		redisFree(redis_context);
	}
}

void *
dave_redis_command(void *context, s8 *command)
{
	redisReply *redis_reply = NULL;
	void *pJson;

	if((context == NULL) || (command == NULL))
	{
		PARTYABNOR("redis input is null!");
		return NULL;
	}

	redis_reply = (redisReply *)redisCommand((redisContext *)context, (const char *)command);

	pJson = _redis_reply_to_json(redis_reply);
	if(pJson == NULL)
	{
		PARTYLOG("empty data. command:%s type:%d", command, redis_reply->type);
	}

	_redis_free_reply(redis_reply);

	return pJson;
}

#endif

