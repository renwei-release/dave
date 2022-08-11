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

static inline ub
_log_save_log_load_key_value(
	s8 **key_start, s8 **key_end, s8 **value_start, s8 **value_end,
	s8 *content_ptr, ub content_len)
{
	ub content_index;

	content_index = 0;

load_key_value_start:
	*key_start = *key_end = *value_start = *value_end = NULL;

	while(content_index < content_len)
	{
		if(content_ptr[content_index] != ' ')
		{
			*key_start = &content_ptr[content_index];
			content_index ++;
			break;
		}
		content_index ++;
	}
	if(*key_start != NULL)
	{
		while(content_index < content_len)
		{
			if(content_ptr[content_index] == ' ')
			{
				if(*key_end == NULL)
				{
					*key_end = &content_ptr[content_index];
				}
			}
			else if(content_ptr[content_index] == ':')
			{
				if(*key_end == NULL)
				{
					*key_end = &content_ptr[content_index];
				}
				content_index ++;
				break;
			}
			else
			{
				if(*key_end != NULL)
				{
					goto load_key_value_start;
				}
			}
			content_index ++;
		}
		while(content_index < content_len)
		{
			if(content_ptr[content_index] != ' ')
			{
				*value_start = &content_ptr[content_index];
				break;
			}
			content_index ++;
		}
		while(content_index < content_len)
		{
			if((content_ptr[content_index] == ' ')
				|| (content_ptr[content_index] == '\n')
				|| (content_ptr[content_index] == '\0'))
			{
				*value_end = &content_ptr[content_index];
				break;
			}
			content_index ++;
		}
		if(content_index == content_len)
		{
			*value_end = &content_ptr[content_index];
		}
	}

	return content_index;
}

// =====================================================================

ub
log_save_load_record(
	s8 *separator, s8 **record_ptr, ub *record_len,
	s8 *content_ptr, ub content_len)
{
	return _log_save_log_load_record(separator, record_ptr, record_len, content_ptr, content_len);
}

ub
log_save_load_key_value(
	s8 **key_ptr, ub *key_len, s8 **value_ptr, ub *value_len,
	s8 *content_ptr, ub content_len)
{
	s8 *key_start, *key_end, *value_start, *value_end;
	ub process_len;

	*key_ptr = *value_ptr = NULL;
	*key_len = *value_len = 0;

	process_len = _log_save_log_load_key_value(
		&key_start, &key_end, &value_start, &value_end,
		content_ptr, content_len);

	if((key_start != NULL) && (key_end != NULL) && (value_start != NULL) && (value_end != NULL))
	{
		*key_ptr = key_start;
		*key_len = (ub)(key_end) - (ub)(key_start);

		*value_ptr = value_start;
		*value_len = (ub)(value_end) - (ub)(value_start);
	}

	return process_len;
}

#endif

