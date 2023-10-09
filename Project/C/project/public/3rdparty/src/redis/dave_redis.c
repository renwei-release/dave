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
#include "dave_tools.h"
#include "party_log.h"

#define REDIS_CONNECT_TIMEOUT_TIME 3

// =====================================================================

/* Synchronous API */

void * dave_redis_connect(s8 *ip, ub port)
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
	        // handle error
	    } 
		else
		{
	        PARTYABNOR("Can't allocate redis context");
		}

		return NULL;
	}

	return (void*)context; 
}

void dave_redis_free_reply(void *reply)
{
	if(reply == NULL)
	{
		PARTYABNOR("redis reply is null!");
		return;
	}
	
	freeReplyObject(reply);
	
	return;
}

void dave_redis_disconnect(void *context)
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

/* Redis command which returns integer value */
ErrCode dave_redis_int_command(void *context, s8 *command, sb *ret_value)
{
	redisReply* redis_reply = NULL;
	ErrCode ret = ERRCODE_OK;

	if(context == NULL || command == NULL || ret_value == NULL)
	{
		PARTYABNOR("redis input is null!");
		return ERRCODE_empty_data;
	}
	
	redis_reply = (redisReply *)redisCommand((redisContext *)context, (const char *)command);

	if(redis_reply == NULL)
	{
		PARTYABNOR("redis reply is null!");
		return ERRCODE_invalid_context;
	}

	if(redis_reply->type == REDIS_REPLY_INTEGER && redis_reply->integer >= 0)
	{
		*ret_value = (sb)redis_reply->integer;
		ret = ERRCODE_OK;
	}
	else if(redis_reply->type == REDIS_REPLY_NIL)
	{
		ret = ERRCODE_empty_data;
	}
	else
	{
		PARTYABNOR("ERROR:redis command:%s, reply:%s", command, redis_reply->str);
		ret = ERRCODE_execute_sql_failed;
	}

	dave_redis_free_reply(redis_reply);

	return ret;
}

#endif

