/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "des_cfg.h"
#include "json_cfg.h"
#include "remote_cfg.h"
#include "cfg_log.h"

static dave_bool
_base_cfg_can_be_set_json_valid_char(u8 valid_char)
{
	if(((valid_char >= 'a') && (valid_char <= 'z'))
		|| ((valid_char >= 'A') && (valid_char <= 'Z'))
		|| ((valid_char >= '0') && (valid_char <= '9'))

		|| (valid_char == '~')
		|| (valid_char == '`')
		|| (valid_char == '!')
		|| (valid_char == '@')
		|| (valid_char == '#')
		|| (valid_char == '$')
		|| (valid_char == '%')
		|| (valid_char == '^')
		|| (valid_char == '&')
		|| (valid_char == '*')
		|| (valid_char == '(')
		|| (valid_char == ')')

		|| (valid_char == '_')
		|| (valid_char == '-')

		|| (valid_char == '+')
		|| (valid_char == '=')

		|| (valid_char == '{')
		|| (valid_char == '[')

		|| (valid_char == '}')
		|| (valid_char == ']')

		|| (valid_char == '|')
		|| (valid_char == '\\')

		|| (valid_char == ':')
		|| (valid_char == ';')

		|| (valid_char == '"')
		|| (valid_char == '\'')

		|| (valid_char == '<')
		|| (valid_char == ',')

		|| (valid_char == '>')
		|| (valid_char == '.')

		|| (valid_char == '?')
		|| (valid_char == '/')

		|| (valid_char == ' '))
	{
		return dave_true;
	}

	return dave_false;
}

static dave_bool
_base_cfg_can_be_set_json_value(u8 *value_ptr, ub value_len)
{
	ub value_index;

	for(value_index=0; value_index<value_len; value_index++)
	{
		if(_base_cfg_can_be_set_json_valid_char(value_ptr[value_index]) == dave_false)
		{
			return dave_false;
		}
	}

	return dave_true;
}

static void
_base_cfg_update(s8 *name, u8 *value_ptr, ub value_len)
{
	CFGUpdate *pUpdate = thread_msg(pUpdate);

	dave_strcpy(pUpdate->cfg_name, name, sizeof(pUpdate->cfg_name));
	if(value_len > sizeof(pUpdate->cfg_value))
	{
		CFGABNOR("name:%s has value_len:%d/%d is long!",
			name, value_len, sizeof(pUpdate->cfg_value));
		value_len = sizeof(pUpdate->cfg_value);
	}
	pUpdate->cfg_length = value_len;
	dave_memcpy(pUpdate->cfg_value, value_ptr, value_len);

	if(broadcast_local(MSGID_CFG_UPDATE, pUpdate) == dave_false)
	{
		CFGABNOR("name:%s update failed!", name);
	}
}

// =====================================================================

void
base_cfg_init(void)
{
	base_remote_cfg_init();
}

void
base_cfg_exit(void)
{
	base_remote_cfg_exit();
}

RetCode
base_cfg_local_set(s8 *dir, s8 *name, u8 *value_ptr, ub value_len)
{
	dave_bool ret;

	if(_base_cfg_can_be_set_json_value(value_ptr, value_len) == dave_true)
	{
		ret = base_json_cfg_dir_set(dir, name, value_ptr, value_len);
	}
	else
	{
		ret = base_des_cfg_dir_set(dir, name, value_ptr, value_len);
	}

	if(ret == RetCode_OK)
	{
		_base_cfg_update(name, value_ptr, value_len);
	}

	return ret;
}

dave_bool
base_cfg_local_get(s8 *dir, s8 *name, u8 *value_ptr, ub value_len)
{
	dave_bool ret;

	dave_memset(value_ptr, 0x00, value_len);

	ret = base_json_cfg_dir_get(dir, name, value_ptr, value_len);
	if(ret == dave_false)
	{
		ret = base_des_cfg_dir_get(dir, name, value_ptr, value_len);
		if(ret == dave_true)
		{
			value_len = dave_strlen(value_ptr);

			if(_base_cfg_can_be_set_json_value(value_ptr, value_len) == dave_true)
			{
				base_cfg_local_set(dir, name, value_ptr, value_len);
			}
		}
	}

	return ret;
}

s8 *
base_cfg_local_get_by_default(s8 *dir, s8 *name, s8 *value_ptr, ub value_len, s8 *default_value)
{
	if(base_cfg_local_get(dir, name, (u8 *)value_ptr, value_len) == dave_false)
	{
		value_len = dave_strcpy(value_ptr, default_value, value_len);
		base_cfg_local_set(dir, name, (u8 *)value_ptr, value_len);
	}

	return value_ptr;
}

RetCode
base_cfg_remote_set(s8 *name, s8 *value, sb ttl)
{
	return base_remote_cfg_set(name, value, ttl);
}

sb
base_cfg_remote_get(s8 *name, s8 *value_ptr, ub value_len)
{
	return base_remote_cfg_get(name, value_ptr, value_len);
}

void
base_cfg_remote_del(s8 *name)
{
	base_remote_cfg_del(name);
}

sb
base_cfg_remote_index(ub index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len)
{
	return base_remote_cfg_index(index, key_ptr, key_len, value_ptr, value_len);
}

dave_bool
base_cfg_remote_internal_add(s8 *name, s8 *value)
{
	return base_remote_cfg_internal_add(name, value);
}

dave_bool
base_cfg_remote_internal_del(s8 *name)
{
	return base_remote_cfg_internal_del(name);
}

RetCode
base_cfg_set_ub(s8 *cfg_name, ub ub_value)
{
	s8 value_ptr[128];
	ub value_len;

	value_len = digitalstring(value_ptr, sizeof(value_ptr), ub_value);

	return cfg_set(cfg_name, (u8 *)value_ptr, value_len);
}

ub
base_cfg_get_ub(s8 *cfg_name)
{
	s8 value_ptr[128];
	ub ub_data;

	if(cfg_get(cfg_name, (u8 *)value_ptr, sizeof(value_ptr)) == dave_true)
	{
		ub_data = stringdigital(value_ptr);
	}
	else
	{
		ub_data = 0;
	}

	return ub_data;
}

RetCode
base_cfg_set_bool(s8 *cfg_name, dave_bool bool_value)
{
	s8 value_ptr[32];
	ub value_len;

	value_len = dave_snprintf(value_ptr, sizeof(value_ptr), "%s", bool_value==dave_true?"true":"false");

	return cfg_set(cfg_name, (u8 *)value_ptr, value_len);
}

dave_bool
base_cfg_get_bool(s8 *cfg_name, dave_bool default_value)
{
	s8 value_ptr[128];

	if(cfg_get(cfg_name, (u8 *)value_ptr, sizeof(value_ptr)) == dave_true)
	{
		lower(value_ptr);
	
		if((dave_strcmp(value_ptr, "true") == dave_true)
			|| (dave_strcmp(value_ptr, "enable") == dave_true))
		{
			return dave_true;
		}
		else
		{
			return dave_false;
		}
	}
	else
	{
		base_cfg_set_bool(cfg_name, default_value);

		return default_value;
	}
}

#endif

