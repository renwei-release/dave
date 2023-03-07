/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "uip_channel.h"
#include "uip_log.h"

static RetCode
_uip_dos_channel_add(s8 *cmd_ptr, ub cmd_len)
{
	s8 channel_name[256];
	s8 *auth_key;

	dos_get_one_parameters(cmd_ptr, cmd_len, channel_name, sizeof(channel_name));
	if(channel_name[0] == '\0')
	{
		return RetCode_Invalid_parameter;
	}

	auth_key = uip_channel_inq(channel_name);
	if(auth_key != NULL)
	{
		dos_print("channel:%s key:%s", channel_name, auth_key);
	}
	else
	{
		auth_key = uip_channel_add(channel_name);
		if(auth_key == NULL)
		{
			dos_print("creat channel:%s failed!", channel_name);	
		}
		else
		{
			dos_print("creat channel:%s key:%s", channel_name, auth_key);
		}
	}

	return RetCode_OK;
}

static RetCode
_uip_dos_channel_inq(s8 *cmd_ptr, ub cmd_len)
{
	s8 channel_name[256];
	s8 *info_ptr;
	ub info_len = 1024 * 16;
	s8 *auth_key;

	dos_get_one_parameters(cmd_ptr, cmd_len, channel_name, sizeof(channel_name));
	if(channel_name[0] == '\0')
	{
		info_ptr = dave_malloc(info_len);
		uip_channel_info(info_ptr, info_len);
		dos_print(info_ptr);
		dave_free(info_ptr);
	}
	else
	{
		auth_key = uip_channel_inq(channel_name);
		if(auth_key == NULL)
		{
			dos_print("can't find channel:%s!", channel_name);	
		}
		else
		{
			dos_print("channel:%s key:%s", channel_name, auth_key);
		}
	}

	return RetCode_OK;
}

static RetCode
_uip_dos_channel_add_method(s8 *cmd_ptr, ub cmd_len)
{
	s8 channel_name[256];
	s8 method[128];
	ub cmd_index;

	cmd_index = dos_get_one_parameters(cmd_ptr, cmd_len, channel_name, sizeof(channel_name));
	dos_get_one_parameters(&cmd_ptr[cmd_index], cmd_len-cmd_index, method, sizeof(method));

	if(uip_channel_add_method(channel_name, method) == dave_false)
		return RetCode_invalid_option;

	dos_print("channel:%s add method:%s", channel_name, method);

	return RetCode_OK;
}

// =====================================================================

void
uip_dos_init(void)
{
	dos_cmd_reg("cadd", _uip_dos_channel_add, NULL);
	dos_cmd_reg("cinq", _uip_dos_channel_inq, NULL);
	dos_cmd_reg("caddm", _uip_dos_channel_add_method, NULL);
}

void
uip_dos_exit(void)
{

}

