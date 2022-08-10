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
	s8 *head;

	content_index = 0;

key_value_find_start:
	*key_start = *key_end = *value_start = *value_end = NULL;
	head = NULL;

	while(content_index < content_len)
	{
		if(content_ptr[content_index] != ' ')
		{
			head = &content_ptr[content_index];
			break;
		}
		content_index ++;
	}
	if(head != NULL)
	{
		while(content_index < content_len)
		{
			if(content_ptr[content_index] == ' ')
			{
				if(*key_start == NULL)
				{
					content_index ++;
					goto key_value_find_start; 
				}
				else
				{
					if(*key_end == NULL)
					{
						*key_end = &content_ptr[content_index];
					}
				}
			}
			else if(content_ptr[content_index] == ':')
			{
				*key_start = head;
				if(*key_end == NULL)
				{
					*key_end = &content_ptr[content_index];
				}
				content_index ++;
				break;
			}
			content_index ++;
		}
		if(*key_start != NULL)
		{
			while(content_index < content_len)
			{
				if(content_ptr[content_index] != ' ')
				{
					*value_start = &content_ptr[content_index];
					break;
				}
				content_index ++;
			}
			if(*value_start != NULL)
			{
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
	s8 **key_start, s8 **key_end, s8 **value_start, s8 **value_end,
	s8 *content_ptr, ub content_len)
{
	return _log_save_log_load_key_value(key_start, key_end, value_start, value_end, content_ptr, content_len);
}

#endif

