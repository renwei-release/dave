/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_http.h"
#include "http_fastcgi.h"
#include "http_log.h"

// =====================================================================

ub
http_copy_uri(s8 *dst_uri, ub dst_uri_len, s8 *src_uri, ub src_uri_len)
{
	ub dst_index, src_index;
	s8 escape[3];

	dst_index = src_index = 0;

	while((dst_index < dst_uri_len) && (src_index < src_uri_len))
	{
		if(src_uri[src_index] == '%')
		{
			src_index ++;

			if((src_index + 2) <= src_uri_len)
			{
				escape[0] = src_uri[src_index ++];
				escape[1] = src_uri[src_index ++];
				escape[2] = '\0';

				dst_uri[dst_index ++] = (s8)stringdigital(escape);
			}
		}
		else
		{
			dst_uri[dst_index ++] = src_uri[src_index ++];
		}
	}

	return dst_index;
}

void
http_copy_head(HttpKeyValue *pDstHead, HttpKeyValue *pSrcHead)
{
	ub head_index = 0;

	if(pDstHead == NULL)
	{
		return;
	}

	if(pSrcHead != NULL)
	{
		for(head_index=0; head_index<DAVE_HTTP_HEAD_MAX; head_index++)
		{
			if(pSrcHead[head_index].key[0] == '\0')
				break;

			dave_strcpy(pDstHead[head_index].key, pSrcHead[head_index].key, DAVE_HTTP_KEY_LEN);
			dave_strcpy(pDstHead[head_index].value, pSrcHead[head_index].value, DAVE_HTTP_VALUE_LEN);
		}
	}

	for(; head_index<DAVE_HTTP_HEAD_MAX; head_index++)
	{
		pDstHead[head_index].key[0] = '\0';
		pDstHead[head_index].value[0] = '\0';
	}
}

s8 *
http_load_content_type(HttpContentType type)
{
	s8 *content_str = NULL;

	switch(type)
	{
		case HttpContentType_json:
				content_str = (s8 *)"Content-Type: application/json\r\n\r\n";
			break;
		case HttpContentType_text:
				content_str = (s8 *)"Content-Type: text/plain;charset=UTF-8\r\n\r\n";
			break;
		default:
				HTTPABNOR("invalid type:%d", type);
			break;
	}

	return content_str;
}

s8 *
http_find_ramkv(HttpKeyValue *head_ptr, ub head_len, char *key)
{
	ub head_index;

	for(head_index=0; head_index<head_len; head_index++)
	{
		if(dave_strcmp(head_ptr[head_index].key, key) == dave_true)
		{
			return head_ptr[head_index].value;
		}
	}

	return NULL;
}

dave_bool
http_build_ramkv(HttpKeyValue *head_ptr, ub head_len, char *key, char *value)
{
	ub empty_index, head_index;

	empty_index = head_len;

	for(head_index=0; head_index<head_len; head_index++)
	{
		if(dave_strcmp(head_ptr[head_index].key, key) == dave_true)
		{
			dave_strcpy(head_ptr[head_index].value, value, DAVE_HTTP_VALUE_LEN);
			return dave_true;
		}

		if(head_ptr[head_index].value[0] == '\0')
		{
			empty_index = head_index;
		}
	}

	if(empty_index >= head_len)
		return dave_false;

	dave_strcpy(head_ptr[empty_index].key, key, DAVE_HTTP_KEY_LEN);
	dave_strcpy(head_ptr[empty_index].value, value, DAVE_HTTP_VALUE_LEN);

	return dave_true;
}

