/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "dos_tools.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

static s8 *
_dos_cfg_dir(s8 *dir_ptr, ub dir_len)
{
	s8 hostname[256], product[256];

	if(dave_os_on_docker() == dave_true)
	{
		dave_os_load_host_name(hostname, sizeof(hostname));
		dave_strfind(hostname, '-', product, sizeof(product));
		dave_snprintf(dir_ptr, dir_len, "/dave/%s", product);
	}
	else
	{
		dave_snprintf(dir_ptr, dir_len, "%s", dave_os_file_home_dir());
	}

	return dir_ptr;
}

static ErrCode
_dos_cfg_get_help(void)
{
	dos_print("Usage: get [config name]\nGet the configuration information, if the configuration name is empty, it is to get all the configuration information.");
	return ERRCODE_OK;
}

static ub
_dos_cfg_get_one(s8 *cmd_ptr, ub cmd_len, s8 *show_ptr, ub show_len)
{
	ub cmd_index, show_index;
	s8 dir[128];
	s8 cfg_name[1024];
	s8 cfg_value[2048];

	cmd_index = show_index = 0;

	while(cmd_index < cmd_len)
	{
		cmd_index += dos_get_str(&cmd_ptr[cmd_index], cmd_len-cmd_index, cfg_name, sizeof(cfg_name));
		if(cfg_name[0] == '\0')
			break;

		dave_memset(cfg_value, 0x00, sizeof(cfg_value));

		if(base_cfg_dir_get(_dos_cfg_dir(dir, sizeof(dir)), cfg_name, (u8 *)cfg_value, sizeof(cfg_value)) == dave_false)
		{
			show_index += dave_snprintf(&show_ptr[show_index], show_len-show_index, "%s:[EMPTY DATA]\n", cfg_name);
		}
		else
		{
			if(t_is_all_show_char((u8 *)cfg_value, dave_strlen(cfg_value)) == dave_true)
			{
				show_index += dave_snprintf(&show_ptr[show_index], show_len-show_index, "%s:%s\n", cfg_name, cfg_value);
			}
			else
			{
				show_index += dave_snprintf(&show_ptr[show_index], show_len-show_index, "%s:[BINARY DATA]\n", cfg_name);
			}
		}
	}

	return show_index;
}

static ub
_dos_cfg_get_all(s8 *show_ptr, ub show_len)
{
	s8 dir[128];
	s8 config_path[1024];
	MBUF *pList, *pConfigKey;
	ub show_index = 0;

	dave_snprintf(config_path, sizeof(config_path), "%s/config", _dos_cfg_dir(dir, sizeof(dir)));

	pList = pConfigKey = dave_os_dir_subdir_list(config_path);

	while(pConfigKey != NULL)
	{
		show_index += _dos_cfg_get_one(
			dave_mptr(pConfigKey), pConfigKey->len,
			&show_ptr[show_index], show_len-show_index);

		pConfigKey = pConfigKey->next;
	}

	dave_mfree(pList);

	return show_index;
}

static ErrCode
_dos_cfg_get(s8 *cmd_ptr, ub cmd_len)
{
	s8 cfg_name[1024];
	s8 show_ptr[8192];
	ub show_len;

	dos_get_str(cmd_ptr, cmd_len, cfg_name, sizeof(cfg_name));
	if(dave_strlen(cfg_name) == 0)
		show_len = _dos_cfg_get_all(show_ptr, sizeof(show_ptr));
	else
		show_len = _dos_cfg_get_one(cfg_name, dave_strlen(cfg_name), show_ptr, sizeof(show_ptr));

	if(show_len == 0)
	{
		dave_snprintf(show_ptr, sizeof(show_ptr), "Empty message!");
	}

	dos_print("%s", show_ptr);

	return ERRCODE_OK;
}

static ErrCode
_dos_cfg_set_help(void)
{
	dos_print("Usage: echo [true]|[false] [thread name]\nStart the echo test to test the link connection performance.!");
	return ERRCODE_OK;
}

static ErrCode
_dos_cfg_set(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index;
	s8 cfg_name[1024];
	s8 cfg_value[2048];
	s8 dir[128];

	cmd_index = 0;

	cmd_index += dos_get_str(&cmd_ptr[cmd_index], cmd_len-cmd_index, cfg_name, sizeof(cfg_name));
	dos_get_str(&cmd_ptr[cmd_index], cmd_len-cmd_index, cfg_value, sizeof(cfg_value));

	if((cfg_name[0] == '\0') || (cfg_value[0] == '\0'))
	{
		return ERRCODE_Invalid_parameter;
	}

	if(base_cfg_dir_set(_dos_cfg_dir(dir, sizeof(dir)), cfg_name, (u8 *)cfg_value, dave_strlen(cfg_value)) != ERRCODE_OK)
	{
		dos_print("%s set %s failed!", cfg_name, cfg_value);
	}
	else
	{
		dos_print("%s set %s success!", cfg_name, cfg_value);
	}

	return ERRCODE_OK;
}

// =====================================================================

void
dos_cfg_reset(void)
{
	dos_cmd_register("get", _dos_cfg_get, _dos_cfg_get_help);
	dos_cmd_register("set", _dos_cfg_set, _dos_cfg_set_help);
}

#endif


