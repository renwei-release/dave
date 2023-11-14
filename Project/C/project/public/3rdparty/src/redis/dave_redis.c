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

RetCode
dave_redis_set(void *context, s8 *set_command)
{
	redisReply* redis_reply = NULL;
	ErrCode ret = ERRCODE_OK;

	if((context == NULL) || (set_command == NULL))
	{
		PARTYABNOR("redis input is null!");
		return RetCode_empty_data;
	}

	redis_reply = (redisReply *)redisCommand((redisContext *)context, (const char *)set_command);

	if(redis_reply == NULL)
	{
		PARTYABNOR("redis reply is null!");
		return RetCode_empty_data;
	}

	if(redis_reply->type == REDIS_REPLY_INTEGER)
	{
		if((redis_reply->integer == 1) || (redis_reply->integer == 0))
		{
			ret = RetCode_OK;
		}
		else
		{
			ret = RetCode_invalid_option;
			PARTYABNOR("%s -> failed:%d", set_command, redis_reply->integer);
		}
	}
	else if(redis_reply->type == REDIS_REPLY_NIL)
	{
		PARTYABNOR("%s -> has invalid ret code", set_command);
		ret = RetCode_invalid_option;
	}
	else
	{
		PARTYABNOR("command:%s, reply:%s", set_command, redis_reply->str);
		ret = RetCode_invalid_option;
	}

	_redis_free_reply(redis_reply);

	return ret;
}

ub
dave_redis_get(void *context, s8 *get_command, u8 *bin_ptr, ub bin_len)
{
	redisReply* redis_reply = NULL;

	if((context == NULL) || (get_command == NULL) || (bin_ptr == NULL) || (bin_len == 0))
	{
		PARTYABNOR("redis input is null!");
		return 0;
	}

	redis_reply = (redisReply *)redisCommand((redisContext *)context, (const char *)get_command);

	if(redis_reply == NULL)
	{
		PARTYABNOR("redis reply is null!");
		return 0;
	}

	if((redis_reply->type == REDIS_REPLY_STRING || redis_reply->type == REDIS_REPLY_STATUS) && (redis_reply->len > 0))
	{
		bin_len = dave_strcpy(bin_ptr, redis_reply->str, bin_len);

		if(redis_reply->len > bin_len)
		{
			PARTYABNOR("reply len:%ld bin_len:%d", redis_reply->len, bin_len);
		}
	}
	else if(redis_reply->type == REDIS_REPLY_NIL)
	{
		bin_len = 0;
	}
	else
	{
		PARTYABNOR("command:%s reply_type:%d reply_str:%s", get_command, redis_reply->type, redis_reply->str);
		bin_len = 0;
	}

	_redis_free_reply(redis_reply);

	return bin_len;
}

RetCode
dave_redis_del(void *context, s8 *del_command)
{
	redisReply* redis_reply = NULL;
	ErrCode ret = ERRCODE_OK;

	if((context == NULL) || (del_command == NULL))
	{
		PARTYABNOR("redis input is null!");
		return RetCode_empty_data;
	}

	redis_reply = (redisReply *)redisCommand((redisContext *)context, (const char *)del_command);

	if(redis_reply == NULL)
	{
		PARTYABNOR("redis reply is null!");
		return RetCode_empty_data;
	}

	if(redis_reply->type == REDIS_REPLY_INTEGER)
	{
		if((redis_reply->integer == 1) || (redis_reply->integer == 0))
		{
			ret = RetCode_OK;
		}
		else
		{
			ret = RetCode_invalid_option;
			PARTYABNOR("%s -> failed:%d", del_command, redis_reply->integer);
		}
	}
	else if(redis_reply->type == REDIS_REPLY_NIL)
	{
		PARTYABNOR("%s -> has invalid ret code", del_command);
		ret = RetCode_invalid_option;
	}
	else
	{
		PARTYABNOR("command:%s reply:%s", del_command, redis_reply->str);
		ret = RetCode_invalid_option;
	}

	_redis_free_reply(redis_reply);

	return ret;
}

#endif

