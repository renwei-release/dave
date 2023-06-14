/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#if defined(LOG_STACK_SERVER) || defined(LOG_STACK_CLIENT)
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "log_save_tools.h"
#include "log_lock.h"
#include "log_log.h"

static inline void
_log_save_json_insert_fun_and_line(void *pJson, s8 *record_ptr, ub record_len)
{
	ub record_index;

	for(record_index=0; record_index<record_len; record_index++)
	{
		if(record_ptr[record_index] == ':')
		{
			dave_json_add_str_len(pJson, "function", record_ptr, record_index);
			record_index ++;
			break;
		}
	}

	if(record_index >= record_len)
		return;

	dave_json_add_str_len(pJson, "line", &record_ptr[record_index], record_len-record_index);
}

static inline void
_log_save_json_insert_receive_date(void *pJson)
{
	DateStruct date;
	s8 date_ptr[128];
	ub date_len;

	t_time_get_date(&date);

	date_len = dave_snprintf(date_ptr, sizeof(date_ptr),
		"%04d.%02d.%02d %02d:%02d:%02d",
		date.year, date.month,
		date.day, date.hour,
		date.minute, date.second);

	dave_json_add_str_len(pJson, "receive_date", date_ptr, date_len);
}

static inline ub
_log_save_json_insert_public(void *pJson, s8 *content_ptr, ub content_len)
{
	s8 separator;
	s8 *record_ptr;
	ub record_len;
	ub content_index;

	content_index = 0;

	content_index += log_save_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	dave_json_add_str_len(pJson, "version", record_ptr, record_len);

	content_index += log_save_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	dave_json_add_str_len(pJson, "build_date", record_ptr, record_len);

	content_index += log_save_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	if(separator == '{')
		dave_json_add_str_len(pJson, "serial", record_ptr, record_len);
	else
		dave_json_add_str_len(pJson, "service", record_ptr, record_len);

	content_index += log_save_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	if(separator == '[')
	{
		dave_json_add_str_len(pJson, "service", record_ptr, record_len);
	}
	if(separator == '<')
	{
		_log_save_json_insert_fun_and_line(pJson, record_ptr, record_len);
	}

	content_index += log_save_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	if(separator == '<')
	{
		_log_save_json_insert_fun_and_line(pJson, record_ptr, record_len);
	}

	return content_index;
}

static inline void
_log_save_json_insert_key_value(void *pJson, s8 *content_ptr, ub content_len)
{
	ub content_index;
	s8 *key_ptr, *value_ptr;
	ub key_len, value_len;
	ub safe_counter;
	s8 backup_key_end;

	content_index = safe_counter = 0;
	while((content_index< content_len) && ((safe_counter ++) < content_len))
	{
		content_index += log_save_load_key_value(
			&key_ptr, &key_len, &value_ptr, &value_len,
			&content_ptr[content_index], content_len-content_index);

		if((key_ptr == NULL)
			|| (key_len == 0)
			|| ((value_ptr == NULL)
			|| (value_len == 0)))
			break;

		backup_key_end = key_ptr[key_len]; key_ptr[key_len] = '\0';

		dave_json_add_str_len(pJson, key_ptr, value_ptr, value_len);

		key_ptr[key_len] = backup_key_end;
	}
}

static inline void
_log_save_json_insert_log_body(void *pJson, s8 *content_ptr, ub content_len)
{
	dave_json_add_str_len(pJson, "log_body", content_ptr, content_len);
}

// =====================================================================

dave_bool
log_save_json(sb file_id, TraceLevel level, s8 *content_ptr, ub content_len)
{
	void *pJson;
	sb file_len, save_len;
	ub process_len;
	s8 *json_str;
	ub json_len;
	dave_bool ret;

	pJson = dave_json_malloc();

	_log_save_json_insert_receive_date(pJson);
	dave_json_add_str(pJson, "level", t_auto_TraceLevel_str(level));
	process_len = _log_save_json_insert_public(pJson, content_ptr, content_len);
	_log_save_json_insert_key_value(pJson, &content_ptr[process_len], content_len-process_len);
	_log_save_json_insert_log_body(pJson, &content_ptr[process_len], content_len-process_len);

	dave_json_add_str_len(pJson, "content", content_ptr, content_len);

	file_len = dave_os_file_len(READ_FLAG, NULL, file_id);
	if(file_len < 0)
	{
		file_len = 0;
	}

	json_str = dave_json_to_string(pJson, &json_len);

	save_len = dave_os_file_save(file_id, (ub)file_len, json_len, (u8 *)json_str);
	if(save_len < (sb)json_len)
	{
		LOGLOG("save file failed:%d/%d", save_len, json_len);
		ret = dave_false;
	}
	else
	{
		file_len += save_len;

		save_len = dave_os_file_save(file_id, (ub)file_len, 1, (u8 *)"\n");
		if(save_len < 1)
		{
			LOGLOG("save file failed:%d/%d", save_len, 1);
			ret = dave_false;
		}
		else
		{
			ret = dave_true;
		}
	}

	dave_json_free(pJson);

	return ret;
}

#endif

