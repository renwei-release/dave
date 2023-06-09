/*
 * Copyright (c) 2023 Renwei
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

#define REMOTE_FILE_NAME "REMOTE.json"

static s8 *
_remote_cfg_file_name(s8 *path_ptr, ub path_len)
{
	dave_snprintf(path_ptr, path_len, "%s/%s/%s", dave_os_file_home_dir(), BASE_CFG_DIR, REMOTE_FILE_NAME);
	return path_ptr;
}

static void
_remote_cfg_file_clean(void)
{
	s8 file_name[1024];

	dave_os_file_remove_dir(_remote_cfg_file_name(file_name, sizeof(file_name)));
}

static void *
_remote_cfg_file_read(void)
{
	s8 file_name[1024];
	void *json;

	json  = dave_json_read(_remote_cfg_file_name(file_name, sizeof(file_name)), dave_true);
	if(json == NULL)
	{
		json = dave_json_malloc();
	}

	return json;
}

static void
_remote_cfg_file_write(void *json)
{
	s8 file_name[1024];

	dave_json_write(json, _remote_cfg_file_name(file_name, sizeof(file_name)), dave_true);

	dave_json_free(json);
}

static struct lh_entry *
_remote_cfg_file_index(void *json, ub index)
{
	struct lh_entry *entry, *find = NULL;
	ub counter = 0;

	if(json == NULL)
		return NULL;

	entry = json_object_get_object(json)->head;
	while(entry)
	{
		if(counter == index)
		{
			find = entry;
			break;
		}
		counter ++;

		entry = entry->next;
	}

	return find;
}

// =====================================================================

void
remote_cfg_file_init(void)
{
	_remote_cfg_file_clean();
}

void
remote_cfg_file_exit(void)
{
	_remote_cfg_file_clean();
}

dave_bool
remote_cfg_file_set(s8 *name, s8 *value)
{
	void *json;
	dave_bool ret;

	json = _remote_cfg_file_read();
	ret = dave_json_add_str(json, name, value);
	_remote_cfg_file_write(json);

	return ret;
}

dave_bool
remote_cfg_file_del(s8 *name)
{
	void *json;

	json = _remote_cfg_file_read();
	CFGDEBUG("%s / %s", name, dave_json_to_string(json, NULL));
	dave_json_del_object(json, name);
	_remote_cfg_file_write(json);

	return dave_true;
}

sb
remote_cfg_file_get(s8 *name, s8 *value_ptr, ub value_len)
{
	void *json;

	json = _remote_cfg_file_read();
	CFGDEBUG("%s / %s", name, dave_json_to_string(json, NULL));
	value_len = dave_json_get_str_v2(json, name, value_ptr, value_len);
	dave_json_free(json);

	return (sb)value_len;
}

sb
remote_cfg_file_index(ub index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len)
{
	void *json;
	struct lh_entry *entry = NULL;
	char *key = NULL;
	struct json_object* val = NULL;
	sb copy_len;

	json = _remote_cfg_file_read();
	CFGDEBUG("%d / %s", index, dave_json_to_string(json, NULL));

	entry = _remote_cfg_file_index(json, index);
	if(entry != NULL)
	{
		key = (char *)entry->k;
		val = (struct json_object *)entry->v;

		dave_strcpy(key_ptr, key, key_len);
		copy_len = dave_strcpy(value_ptr, json_object_get_string(val), value_len);
	}
	else
	{
		copy_len = -1;
	}

	dave_json_free(json);

	return copy_len;
}

#endif

