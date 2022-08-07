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
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "dos_tools.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

static RetCode
_dos_cfg_get_help(void)
{
	dos_print("Usage: get [config name]\nGet the configuration information, if the configuration name is empty, it is to get all the configuration information.");
	return RetCode_OK;
}

static ub
_dos_cfg_get_one(s8 *cmd_ptr, ub cmd_len, s8 *show_ptr, ub show_len)
{
	ub cmd_index, show_index;
	s8 cfg_name[1024];
	s8 cfg_value[2048];

	cmd_index = show_index = 0;

	while(cmd_index < cmd_len)
	{
		cmd_index += dos_load_string(&cmd_ptr[cmd_index], cmd_len-cmd_index, cfg_name, sizeof(cfg_name));
		if(cfg_name[0] == '\0')
			break;

		dave_memset(cfg_value, 0x00, sizeof(cfg_value));

		if(cfg_get(cfg_name, (u8 *)cfg_value, sizeof(cfg_value)) == dave_false)
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
_dos_cfg_get_dir_all(s8 *show_ptr, ub show_len, void *pJson)
{
	s8 config_path[1024];
	s8 json_data[128];
	ub json_len;
	MBUF *pList, *pConfigKey;
	ub show_index = 0;

	dave_snprintf(config_path, sizeof(config_path), "%s/config", dave_os_file_home_dir());

	pList = pConfigKey = dave_os_dir_subdir_list(config_path);

	while(pConfigKey != NULL)
	{
		json_len = sizeof(json_data);

		if(dave_json_get_str(pJson, (char *)dave_mptr(pConfigKey), json_data, &json_len) == dave_false)
		{
			show_index += _dos_cfg_get_one(
				dave_mptr(pConfigKey), dave_mlen(pConfigKey),
				&show_ptr[show_index], show_len-show_index);
		}

		pConfigKey = pConfigKey->next;
	}

	dave_mfree(pList);

	return show_index;
}

static ub
_dos_cfg_get_json_all(s8 *show_ptr, ub show_len, void **ppJson)
{
	s8 config_json_path[1024];
	ub data_len = 3 * 1024 * 1024;
	ub read_len;
	u8 *data_ptr;
	void *pJson = NULL;

	data_ptr = dave_malloc(data_len);

	dave_snprintf(config_json_path, sizeof(config_json_path), "%s/config/CONFIG.json", dave_os_file_home_dir());

	read_len = dave_os_file_read(DIRECT_FLAG|READ_FLAG, config_json_path, 0, data_len, data_ptr);
	if(read_len < data_len)
		data_ptr[read_len] = '\0';

	if(read_len > 0)
	{
		pJson = dave_string_to_json(data_ptr, read_len);
	}

	if(pJson != NULL)
	{
		show_len = dave_snprintf(show_ptr, show_len, "%s\n\n", dave_json_to_string(pJson, NULL));
	}
	else
	{
		show_len = 0;
	}

	*ppJson = pJson;

	return show_len;
}

static ub
_dos_cfg_get_all(s8 *show_ptr, ub show_len)
{
	ub show_index = 0;
	void *pJson;

	show_index += _dos_cfg_get_json_all(&show_ptr[show_index], show_len-show_index, &pJson);
	show_index += _dos_cfg_get_dir_all(&show_ptr[show_index], show_len-show_index, pJson);

	dave_json_free(pJson);

	return show_index;
}

static RetCode
_dos_cfg_get(s8 *cmd_ptr, ub cmd_len)
{
	s8 cfg_name[1024];
	s8 show_ptr[8192];
	ub show_len;

	dos_load_string(cmd_ptr, cmd_len, cfg_name, sizeof(cfg_name));
	if(dave_strlen(cfg_name) == 0)
		show_len = _dos_cfg_get_all(show_ptr, sizeof(show_ptr));
	else
		show_len = _dos_cfg_get_one(cfg_name, dave_strlen(cfg_name), show_ptr, sizeof(show_ptr));

	if(show_len == 0)
	{
		dave_snprintf(show_ptr, sizeof(show_ptr), "Empty message!");
	}

	dos_print("%s", show_ptr);

	return RetCode_OK;
}

static RetCode
_dos_cfg_set(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index;
	s8 cfg_name[1024];
	s8 cfg_value[2048];

	cmd_index = 0;

	cmd_index += dos_load_string(&cmd_ptr[cmd_index], cmd_len-cmd_index, cfg_name, sizeof(cfg_name));
	dos_get_last_parameters(&cmd_ptr[cmd_index], cmd_len-cmd_index, cfg_value, sizeof(cfg_value));

	if((cfg_name[0] == '\0') || (cfg_value[0] == '\0'))
	{
		return RetCode_Invalid_parameter;
	}

	if(cfg_set(cfg_name, (u8 *)cfg_value, dave_strlen(cfg_value)) != RetCode_OK)
	{
		dos_print("%s set %s failed!", cfg_name, cfg_value);
	}
	else
	{
		dos_print("%s set %s success!", cfg_name, cfg_value);
	}

	return RetCode_OK;
}

static void
_dos_cfg_remote_get_one(s8 *cfg_name)
{
	s8 cfg_value[2048];

	if(rcfg_get(cfg_name, cfg_value, sizeof(cfg_value)) == 0)
	{
		dos_print("%s remote get failed!", cfg_name);
	}
	else
	{
		dos_print("%s : %s", cfg_name, cfg_value);
	}
}

static void
_dos_cfg_remote_get_all(void)
{
	ub index;
	s8 cfg_name[256];
	s8 cfg_value[2048];
	dave_bool empty_flag = dave_true;

	for(index=0; index<102400; index++)
	{
		if(rcfg_index(index, cfg_name, sizeof(cfg_name), cfg_value, sizeof(cfg_value)) == 0)
		{
			break;
		}

		dos_print("%s : %s", cfg_name, cfg_value);

		empty_flag = dave_false;
	}

	if(empty_flag == dave_true)
	{
		dos_print("Empty message!");
	}
}

static RetCode
_dos_cfg_remote_get(s8 *cmd_ptr, ub cmd_len)
{
	s8 cfg_name[1024];

	dos_load_string(cmd_ptr, cmd_len, cfg_name, sizeof(cfg_name));
	if(dave_strlen(cfg_name) == 0)
	{
		_dos_cfg_remote_get_all();
	}
	else
	{
		_dos_cfg_remote_get_one(cfg_name);
	}

	return RetCode_OK;
}

static RetCode
_dos_cfg_remote_set(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index;
	s8 cfg_name[1024];
	s8 cfg_value[2048];

	cmd_index = 0;

	cmd_index += dos_load_string(&cmd_ptr[cmd_index], cmd_len-cmd_index, cfg_name, sizeof(cfg_name));
	dos_get_last_parameters(&cmd_ptr[cmd_index], cmd_len-cmd_index, cfg_value, sizeof(cfg_value));

	if((cfg_name[0] == '\0') || (cfg_value[0] == '\0'))
	{
		return RetCode_Invalid_parameter;
	}

	if(rcfg_set(cfg_name, cfg_value) != RetCode_OK)
	{
		dos_print("%s remote set %s failed!", cfg_name, cfg_value);
	}
	else
	{
		dos_print("%s remote set %s success!", cfg_name, cfg_value);
	}

	return RetCode_OK;
}

// =====================================================================

void
dos_cfg_reset(void)
{
	dos_cmd_reg("get", _dos_cfg_get, _dos_cfg_get_help);
	dos_cmd_reg("set", _dos_cfg_set, NULL);
	dos_cmd_reg("rget", _dos_cfg_remote_get, NULL);
	dos_cmd_reg("rset", _dos_cfg_remote_set, NULL);
}

#endif


