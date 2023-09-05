/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "uip_server_register.h"
#include "uip_server_monitor.h"
#include "uip_server_http.h"
#include "uip_server_send.h"
#include "uip_server_recv.h"
#include "uip_channel.h"
#include "uip_server_wechat.h"
#include "uip_parsing.h"
#include "uip_log.h"

static char *_uip_head_key[] = {
	UIP_JSON_METHOD,
	UIP_JSON_CHANNEL,
	UIP_JSON_AUTH_KEY,
	UIP_JSON_SERIAL,
	UIP_JSON_CURRENT_MILLISECONDS,
	UIP_JSON_RESULT_CODE,
	UIP_JSON_RESULT_DESC,
	NULL
} ;

// =====================================================================

dave_bool
uip_is_head(s8 *key)
{
	ub key_index;

	for(key_index=0; key_index<4096; key_index++)
	{
		if(_uip_head_key[key_index] == NULL)
		{
			return dave_false;
		}

		if(dave_strcmp(_uip_head_key[key_index], key) == dave_true)
		{
			return dave_true;
		}
	}

	return dave_false;
}

void
uip_write_stack(char *stack_name, UIPStack *pStack)
{
	s8 file_name[256];
	void *pJson;

	dave_snprintf(file_name, sizeof(file_name),
		"./%s_%s.txt",
		pStack->head.method, stack_name);

	if(pStack->auto_release_json == NULL)
	{
		pJson = uip_encode(pStack, dave_true);
		dave_json_write(pJson, file_name, dave_true);
		dave_json_free(pJson);
	}
	else
	{
		dave_json_write(pStack->auto_release_json, file_name, dave_true);
	}
}

sb
uip_ret_to_code(RetCode ret)
{
	if(ret == RetCode_OK)
		return 1;
	else
		return ret;
}

s8 *
uip_ret_to_desc(RetCode ret)
{
	return retstr(ret);
}

