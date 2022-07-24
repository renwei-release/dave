/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "log_lock.h"
#include "log_log.h"

static const s8 _content_separator[][2] = {
	{'(', ')'},
	{'<', '>'},
	{'{', '}'},
	{'[', ']'},
	{0, 0}
};

static inline dave_bool
_log_save_log_is_start_separator(s8 content)
{
	ub separator_index;

	for(separator_index=0; separator_index<128; separator_index++)
	{
		if(_content_separator[separator_index][0] == 0)
			break;

		if(_content_separator[separator_index][0] == content)
			return dave_true;
	}

	return dave_false;
}

static inline dave_bool
_log_save_log_is_end_separator(s8 content)
{
	ub separator_index;

	for(separator_index=0; separator_index<128; separator_index++)
	{
		if(_content_separator[separator_index][1] == 0)
			break;

		if(_content_separator[separator_index][1] == content)
			return dave_true;
	}

	return dave_false;
}

static inline ub
_log_save_log_load_record(s8 *separator, s8 **record_ptr, ub *record_len, s8 *content_ptr, ub content_len)
{
	ub content_index;
	s8 *load_record_ptr;
	ub load_record_len;

	*separator = '(';
	*record_ptr = NULL;
	*record_len = 0;

	content_index = 0;

	while(content_index < content_len)
	{
		if(_log_save_log_is_start_separator(content_ptr[content_index]) == dave_true)
		{
			*separator = content_ptr[content_index];
			content_index ++;
			break;
		}
		content_index ++;
	}
	if(content_index >= content_len)
	{
		return content_index;
	}

	load_record_ptr = &content_ptr[content_index];
	load_record_len = 0;

	while(content_index < content_len)
	{
		if(_log_save_log_is_end_separator(content_ptr[content_index]) == dave_true)
		{
			*record_ptr = load_record_ptr;
			*record_len = load_record_len;

			content_index ++;
			break;
		}

		load_record_len ++;
		content_index ++;
	}

	if(*record_ptr == NULL)
		return 0;

	return content_index;
}

static inline void
_log_save_log_insert_fun_and_line(void *pJson, s8 *record_ptr, ub record_len)
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
_log_save_log_insert_receive_date(void *pJson)
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
_log_save_log_insert_content(void *pJson, s8 *content_ptr, ub content_len)
{
	s8 separator;
	s8 *record_ptr;
	ub record_len;
	ub content_index;

	content_index = 0;

	content_index += _log_save_log_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	dave_json_add_str_len(pJson, "version", record_ptr, record_len);

	content_index += _log_save_log_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	dave_json_add_str_len(pJson, "build_date", record_ptr, record_len);

	content_index += _log_save_log_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	if(separator == '{')
		dave_json_add_str_len(pJson, "serial", record_ptr, record_len);
	else
		dave_json_add_str_len(pJson, "service", record_ptr, record_len);

	content_index += _log_save_log_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
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
		_log_save_log_insert_fun_and_line(pJson, record_ptr, record_len);
	}

	content_index += _log_save_log_load_record(&separator, &record_ptr, &record_len, &content_ptr[content_index], content_len-content_index);
	if((record_ptr == NULL) || (record_len == 0))
	{
		return content_index;
	}
	if(separator == '<')
	{
		_log_save_log_insert_fun_and_line(pJson, record_ptr, record_len);
	}

	return content_index;
}

// =====================================================================

void
log_save_log(sb file_id, TraceLevel level, s8 *content_ptr, ub content_len)
{
	void *pJson;
	sb file_len;
	ub process_len;
	s8 *json_str;
	ub json_len;

	pJson = dave_json_malloc();

	_log_save_log_insert_receive_date(pJson);
	dave_json_add_str(pJson, "level", t_auto_TraceLevel_str(level));
	process_len = _log_save_log_insert_content(pJson, content_ptr, content_len);
	if(process_len == 0)
	{
		dave_json_add_str_len(pJson, "log_body", content_ptr, content_len);
	}
	else if(process_len < content_len)
	{
		dave_json_add_str_len(pJson, "log_body", &content_ptr[process_len], content_len-process_len);
	}
	dave_json_add_str_len(pJson, "content", content_ptr, content_len);

	file_len = dave_os_file_len(NULL, file_id);
	if(file_len < 0)
	{
		file_len = 0;
	}

	json_str = dave_json_to_string(pJson, &json_len);

	file_len += dave_os_file_save(file_id, (ub)file_len, json_len, (u8 *)json_str);
	dave_os_file_save(file_id, (ub)file_len, 1, (u8 *)"\n");

	dave_json_free(pJson);
}

#endif

