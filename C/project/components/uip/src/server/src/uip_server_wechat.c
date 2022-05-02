/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "uip_server_register.h"
#include "uip_server_http.h"
#include "uip_key_define.h"
#include "uip_parsing.h"
#include "uip_tools.h"
#include "uip_log.h"

#define UIP_WECHAT_KEY_VALUE_MAX (1024)

static ub
_uip_server_wechat_h5_data_change(u8 *h5_data, ub h5_length)
{
	ub h5_index, change_index;
	s8 temp_buf[16];

	h5_index = change_index = 0;

	while(h5_index < h5_length)
	{
		if(h5_data[h5_index] == '%')
		{
			if((h5_index + 3) > h5_length)
			{
				UIPABNOR("invalid h5_index:%d h5_length:%d", h5_index, h5_length);
				break;
			}

			dave_memcpy(temp_buf, &h5_data[h5_index + 1], 2);
			temp_buf[2] = '\0';
			h5_index += 3;

			h5_data[change_index ++] = (u8)stringhexdigital(temp_buf);
		}
		else
		{
			h5_data[change_index ++] = h5_data[h5_index ++];
		}
	}

	if(change_index < h5_length)
	{
		h5_data[change_index] = '\0';
	}
	else
	{
		UIPABNOR("invalid h5_index:%d h5_length:%d", h5_index, h5_length);
	}

	return change_index;
}

static s8 *
_uip_server_wechat_load_h5_content(s8 *http_data, ub http_length, ub *content_length)
{
	ub content_start_index;

	http_length = _uip_server_wechat_h5_data_change((u8 *)http_data, http_length);

	UIPDEBUG("http_data:%s", http_data);

	content_start_index = 0;

	while(content_start_index < http_length)
	{
		if(dave_memcmp(&http_data[content_start_index], "base64,", 7) == dave_true)
		{
			http_data[content_start_index] = '\0';
			content_start_index += 7;
			break;
		}

		content_start_index ++;
	}

	if(content_start_index >= http_length)
	{
		UIPLOG("can't find content:%s", http_data);
		return NULL;
	}

	*content_length = http_length - content_start_index;

	return &http_data[content_start_index];
}

static s8 *
_uip_server_wechat_load_weichat_content(s8 *http_data, ub http_length, ub *content_length)
{
	ub index;

	*content_length = 0;

	index = 0;

	while(index < http_length)
	{
		if(dave_memcmp(&http_data[index], "Content-Disposition: form-data; name=\"file\"; filename=", 54) == dave_true)
		{
			break;
		}

		index ++;
	}

	if(index >= http_length)
	{
		UIPLOG("invalid content:%s", http_data);
		return NULL;
	}

	while(index < http_length)
	{
		if((http_data[index] == '\r')
			&& (http_data[index+1] == '\n')
			&& (http_data[index+2] == '\r')
			&& (http_data[index+3] == '\n'))
		{
			http_data[index] = '\0';
			index += 4;
			break;
		}

		index ++;
	}

	if(index >= http_length)
	{
		UIPLOG("invalid content:%s", http_data);
		return NULL;
	}

	*content_length = http_length - index;

	return &http_data[index];
}

static UIPType
_uip_server_wechat_data_type(s8 *data_ptr)
{
	if(data_ptr[0] == '{')
	{
		return UIPType_json;
	}
	else if(dave_memcmp(data_ptr, "command=", 8) == dave_true)
	{
		return UIPType_h5_form;
	}
	else
	{
		return UIPType_weichat_form;
	}
}

static UIPType
_uip_server_wechat_load_content(s8 *http_data, ub http_length, s8 **content, ub *content_length)
{
	UIPType type;

	type = _uip_server_wechat_data_type(http_data);

	switch(type)
	{
		case UIPType_json:
				*content = http_data;
				*content_length = http_length;
			break;
		case UIPType_h5_form:
				*content = _uip_server_wechat_load_h5_content(http_data, http_length, content_length);
			break;
		case UIPType_weichat_form:
				*content = _uip_server_wechat_load_weichat_content(http_data, http_length, content_length);
			break;
		default:
				*content = NULL;
				*content_length = 0;
			break;
	}

	return type;
}

static ub
_uip_server_weichat_form_key_value(s8 *head, s8 *key, ub key_len, s8 *value, ub value_len)
{
	ub index, safe_counter, key_index, value_index;

	key[0] = value[0] = '\0';

	safe_counter = index = 0;
	while((++ safe_counter) < 4096)
	{
		if(head[index] == '\0')
		{
			break;
		}

		if(dave_memcmp(&head[index], "Content-Disposition: form-data; name=\"", 38) == dave_true)
		{
			index += 38;
			break;
		}

		index ++;
	}
	if((head[index] == '\0') || (safe_counter >= 4096))
	{
		return index;
	}

	safe_counter = key_index = 0;
	while(((++ safe_counter) < 4096) && (key_index < (key_len - 1)))
	{
		if(head[index] == '\0')
		{
			break;
		}

		if(head[index] == '"')
		{
			index ++;
			break;
		}

		key[key_index ++] = head[index ++];
	}
	key[key_index] = '\0';
	if((head[index] == '\0') || (safe_counter >= 4096))
	{
		return index;
	}

	safe_counter = 0;
	while((++ safe_counter) < 4096)
	{
		if(head[index] == '\0')
		{
			break;
		}

		if((head[index] != '\r') && (head[index] != '\n') && (head[index] != ' '))
		{
			break;
		}

		index ++;
	}
	if((head[index] == '\0') || (safe_counter >= 4096))
	{
		return index;
	}

	safe_counter = value_index = 0;
	while(((++ safe_counter) < 4096) && (value_index < (value_len - 1)))
	{
		if(head[index] == '\0')
		{
			break;
		}

		if((head[index] == '\r') || (head[index] == '\n') || ((head[index] == '-') && (head[index+1] == '-')))
		{
			index ++;
			break;
		}

		value[value_index ++] = head[index ++];
	}
	value[value_index] = '\0';

	return index;
}

static void *
_uip_server_wechat_form_json_decode(s8 *string_ptr, ub string_length, UIPType type)
{
	ub index, safe_counter;
	s8 key[1024];
	s8 value[1024];
	void *pJson, *pHead, *pBody;

	index = safe_counter = 0;

	pJson = dave_json_malloc();
	pHead = dave_json_malloc();
	pBody = dave_json_malloc();

	while(((++ safe_counter) < UIP_WECHAT_KEY_VALUE_MAX) && (index < string_length))
	{
		if(type == UIPType_weichat_form)
		{
			index += _uip_server_weichat_form_key_value(&string_ptr[index], key, sizeof(key), value, sizeof(value));
		}
		if(key[0] == '\0')
		{
			break;
		}

		if(uip_is_head(key) == dave_true)
		{
			dave_json_add_str(pHead, (char *)key, value);
		}
		else
		{
			dave_json_add_str(pBody, (char *)key, value);
		}
	}

	if(safe_counter > UIP_WECHAT_KEY_VALUE_MAX)
	{
		UIPABNOR("load max head!");
	}

	dave_json_add_ub(pJson, UIP_JSON_VERSION, UIP_VERSION);
	dave_json_add_object(pJson, UIP_JSON_HEAD, pHead);
	dave_json_add_object(pJson, UIP_JSON_BODY, pBody);

	return pJson;
}

static void
_uip_server_wechat_form_body_encode(void *pJson, s8 *content_ptr, ub content_length)
{
	ub data_length, base64_length;
	s8 *data_str;

	data_length = content_length * 4;
	data_str = dave_malloc(data_length);

	base64_length = t_crypto_base64_encode((const u8 *)(content_ptr), content_length, data_str, data_length);
	if(base64_length == 0)
	{
		UIPABNOR("base64_length is zero!");
	}

	UIPDEBUG("base64_length:%d", base64_length);

	dave_json_add_str(pJson, UIP_JSON_BODY_DATA, data_str);

	dave_free(data_str);
}

static void *
_uip_server_wechat_form_decode(UIPType type, s8 *string_ptr, ub string_length, s8 *content_ptr, ub content_length)
{
	void *pJson, *pBody;

	pJson = _uip_server_wechat_form_json_decode(string_ptr, string_length, type);

	if((content_ptr != NULL) && (content_length > 0))
	{
		pBody = dave_json_get_object(pJson, UIP_JSON_BODY);
		if(pBody == NULL)
		{
			pBody = dave_json_malloc();

			dave_json_add_object(pJson, UIP_JSON_BODY, pBody);
		}

		_uip_server_wechat_form_body_encode(pBody, content_ptr, content_length);
	}

	return pJson;
}

// =====================================================================

void *
uip_server_wechat(s8 *string_ptr, ub string_length)
{
	void *pJson;
	UIPType type;
	s8 *content_ptr;
	ub content_length;

	type = _uip_server_wechat_load_content(string_ptr, string_length, &content_ptr, &content_length);

	UIPDEBUG("type:%d content_length:%d %d/%s", type, content_length, string_length, string_ptr);

	switch(type)
	{
		case UIPType_weichat_form:
				pJson = _uip_server_wechat_form_decode(type, string_ptr, string_length-content_length, content_ptr, content_length);
			break;
		default:
				pJson = NULL;
			break;
	}

	return pJson;
}

