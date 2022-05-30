/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "cfg_param.h"
#include "cfg_log.h"

static volatile sb _json_config_init_ = 0;
static TLock _json_config_option_pv;

static inline void
_base_json_cfg_booting(void)
{
	t_lock_spin(NULL);

	if(_json_config_init_ != 0x89807abcd)
	{
		_json_config_init_ = 0x89807abcd;

		t_lock_reset(&_json_config_option_pv);
	}

	t_unlock_spin(NULL);
}

static inline FileOptFlag
_base_json_cfg_file_path(s8 *dir, s8 *path_ptr, ub path_len)
{
	FileOptFlag flag;
	const s8 *cfg_file_name = "CONFIG.json";

	if(dir == NULL)
	{
		flag = CREAT_READ_WRITE_FLAG;
		dave_snprintf(path_ptr, path_len, "%s/%s", BASE_CFG_DIR, cfg_file_name);
	}
	else
	{
		flag = DIRECT_FLAG | CREAT_READ_WRITE_FLAG;
		dave_snprintf(path_ptr, path_len, "%s/%s/%s", dir, BASE_CFG_DIR, cfg_file_name);
	}

	CFGDEBUG("flag:%x dir:%s path:%s", flag, dir, path_ptr);

	return flag;
}

static void *
_base_json_cfg_read(FileOptFlag flag, s8 *file_name)
{
	ub data_len = 3 * 1024 * 1024;
	ub read_len;
	u8 *data_ptr;
	void *pJson = NULL;

	data_ptr = dave_malloc(data_len);

	read_len = dave_os_file_read(flag, file_name, 0, data_len, data_ptr);
	if(read_len > 0)
	{
		if(read_len < data_len)
			data_ptr[read_len] = '\0';

		pJson = dave_string_to_json(data_ptr, read_len);
		if(pJson == NULL)
		{
			CFGABNOR("flag:%x file_name:%s is invalid json data!",
				flag, file_name);
		}
	}

	if(pJson == NULL)
	{
		pJson = dave_json_malloc();
	}

	dave_free(data_ptr);

	return pJson;
}

static dave_bool
_base_json_cfg_write(FileOptFlag flag, s8 *file_name, void *pJson)
{
	ub data_len;
	u8 *data_ptr;

	data_ptr = (u8 *)dave_json_to_string(pJson, &data_len);

	dave_os_file_delete(flag, file_name);

	return dave_os_file_write(flag, file_name, 0, data_len, data_ptr);
}

static RetCode
_base_json_cfg_dir_set(s8 *dir, s8 *name, u8 *value_ptr, ub value_len)
{
	s8 file_name[1024];
	FileOptFlag flag;
	void *pJson;

	flag = _base_json_cfg_file_path(dir, file_name, sizeof(file_name));

	pJson = _base_json_cfg_read(flag, file_name);

	if(dave_json_add_str_len(pJson, name, (s8 *)value_ptr, value_len) == dave_false)
	{
		CFGABNOR("%s add failed!", name);
	}

	_base_json_cfg_write(flag, file_name, pJson);

	dave_json_free(pJson);

	return RetCode_OK;
}

static dave_bool
_base_json_cfg_dir_get(s8 *dir, s8 *name, u8 *value_ptr, ub value_len)
{
	s8 file_name[1024];
	FileOptFlag flag;
	void *pJson;
	dave_bool ret;

	flag = _base_json_cfg_file_path(dir, file_name, sizeof(file_name));

	pJson = _base_json_cfg_read(flag, file_name);

	ret = dave_json_get_str(pJson, name, value_ptr, &value_len);

	dave_json_free(pJson);

	return ret;
}

// =====================================================================

RetCode
base_json_cfg_dir_set(s8 *dir, s8 *name, u8 *value_ptr, ub value_len)
{
	RetCode ret = RetCode_Parameter_conflicts;

	_base_json_cfg_booting();

	SAFECODEv1(_json_config_option_pv, ret = _base_json_cfg_dir_set(dir, name, value_ptr, value_len); );

	return ret;
}

dave_bool
base_json_cfg_dir_get(s8 *dir, s8 *name, u8 *value_ptr, ub value_len)
{
	dave_bool ret = dave_false;

	_base_json_cfg_booting();

	SAFECODEv1(_json_config_option_pv, ret = _base_json_cfg_dir_get(dir, name, value_ptr, value_len); );

	return ret;
}

#endif

