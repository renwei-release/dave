/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_http.h"
#include "http_recv.h"
#include "http_recv_param.h"
#include "http_fastcgi.h"
#include "http_tools.h"
#include "http_log.h"

#define FASTCGI_STDIN_MAX (HTTP_RECV_BUF_MAX)
#define FASTCGI_STDOUT_MAX (65535)

static dave_bool
_http_fastcgi_parse_head(u8 *data_ptr, ub data_len, ub *head_len, Fcgi *pFcgi)
{
	ub data_index;

	if(data_len < 8)
	{
		HTTPABNOR("invalid data_len:%d pFcgi:%lx", data_len, pFcgi);
		*head_len = data_len;
		return dave_true;
	}

	data_index = 0;

	pFcgi->head.version = (ub)(data_ptr[data_index ++]);
	if(pFcgi->head.version != FCGI_VERSION_1)
	{	
		HTTPLOG("invalid version:%d %d/%d pFcgi:%lx", pFcgi->head.version, data_index, data_len, pFcgi);
		*head_len = data_len;
		return dave_true;
	}
	pFcgi->head.type = (FcgiType)(data_ptr[data_index ++]);
	if((pFcgi->head.type < FCGI_BEGIN_REQUEST) || (pFcgi->head.type > FCGI_UNKNOWN_TYPE))
	{	
		HTTPLOG("invalid type:%d %d/%d pFcgi:%lx", pFcgi->head.type, data_index, data_len, pFcgi);
		*head_len = data_len;
		return dave_true;
	}
	pFcgi->head.request_id = (((ub)(data_ptr[data_index])) << 8) + ((ub)(data_ptr[data_index + 1]));
	if(pFcgi->head.request_id != 1)
	{
		HTTPLOG("found a request requence number[%ld] that is not 1! version:%d type:%d %d/%d pFcgi:%lx",
			pFcgi->head.request_id,
			pFcgi->head.version,
			pFcgi->head.type,
			data_index, data_len,
			pFcgi);
		*head_len = data_len;
		return dave_true;
	}
	data_index += 2;
	pFcgi->head.content_length = (((ub)(data_ptr[data_index])) << 8) + ((ub)(data_ptr[data_index + 1]));
	data_index += 2;
	pFcgi->head.padding_length = (ub)(data_ptr[data_index ++]);
	pFcgi->head.reserved = (ub)(data_ptr[data_index ++]);

	*head_len = data_index;
	return dave_false;
}

static ub
_http_fastcgi_build_head(u8 *data_ptr, ub data_len, FcgiType type, ub request_id, ub content_length, ub padding_length)
{
	ub data_index;

	if(data_len < 8)
	{
		HTTPABNOR("invalid data-len:%d", data_len);
		return 0;
	}

	data_index = 0;

	data_ptr[data_index ++] = FCGI_VERSION_1;
	data_ptr[data_index ++] = (u8)type;
	data_ptr[data_index ++] = (u8)(request_id >> 8);
	data_ptr[data_index ++] = (u8)(request_id);
	data_ptr[data_index ++] = (u8)(content_length >> 8);
	data_ptr[data_index ++] = (u8)(content_length);
	data_ptr[data_index ++] = (u8)(padding_length);
	data_ptr[data_index ++] = 0x00;

	return data_index;
}

static void
_http_fastcgi_parse_begin_request(u8 *data_ptr, ub data_len, Fcgi *pFcgi)
{
	ub data_index;
	FCGIBeginRequestBody *pBody = &(pFcgi->request);

	if(pFcgi->head.content_length != 8)
	{
		HTTPABNOR("invalid content length:%d", pFcgi->head.content_length);
	}

	data_index = 0;

	pBody->role = (((ub)(data_ptr[data_index])) << 8) + ((ub)(data_ptr[data_index + 1]));
	data_index += 2;
	pBody->flags = (FCGIFlags)(data_ptr[data_index ++]);
	data_index += 5;

	if(pBody->role != FCGI_RESPONDER)
	{
		HTTPABNOR("unsupport role:%d", pBody->role);
	}

	if((pBody->flags != FCGI_DONOT_KEEP_CONN) && (pBody->flags != FCGI_KEEP_CONN))
	{
		HTTPABNOR("unsupport flags:%d", pBody->flags);
	}

	if(pFcgi->head.content_length != data_index)
	{
		HTTPABNOR("has content not process:%d/%d!", data_index, pFcgi->head.content_length);
	}
}

static void
_http_fastcgi_free_param(FCGIParamsBody *param)
{
	if(param != NULL)
	{
		dave_free(param->name);
		dave_free(param->value);

		dave_free(param);
	}
}

static FCGIParamsBody *
_http_fastcgi_malloc_param(ub name_length, ub value_length)
{
	FCGIParamsBody *param;

	param = dave_malloc(sizeof(FCGIParamsBody));
	param->name = dave_malloc(name_length + 1);
	param->value = dave_malloc(value_length + 1);
	param->next = NULL;

	return param;
}

static ub
_http_fastcgi_parse_param_(u8 *data_ptr, ub data_len, Fcgi *pFcgi)
{
	ub data_index;
	ub name_length, value_length;
	FCGIParamsBody *param, *end_param;
	ub safe_counter;

	if(data_len < 2)
	{
		HTTPLOG("invalid data_len:%d", data_len);
		return data_len;
	}

	data_index = 0;

	if(data_ptr[data_index] & 0x80)
	{
		name_length = (((ub)(data_ptr[data_index]&0x7f)) << 24) + (((ub)(data_ptr[data_index + 1])) << 16) + (((ub)(data_ptr[data_index + 2])) << 8) + (((ub)(data_ptr[data_index + 3])));
		data_index += 4;
	}
	else
	{
		name_length = (ub)data_ptr[data_index ++];
	}
	if(data_ptr[data_index] & 0x80)
	{
		value_length = (((ub)(data_ptr[data_index]&0x7f)) << 24) + (((ub)(data_ptr[data_index + 1])) << 16) + (((ub)(data_ptr[data_index + 2])) << 8) + (((ub)(data_ptr[data_index + 3])));
		data_index += 4;
	}
	else
	{
		value_length = (ub)data_ptr[data_index ++];
	}

	if((data_index + name_length + value_length) > data_len)
	{
		HTTPLOG("invalid name-length:%ld value-length:%ld data-len:%ld",
			name_length, value_length, data_len);
		return data_len;
	}

	param = _http_fastcgi_malloc_param(name_length, value_length);

	dave_strcpy(param->name, &data_ptr[data_index], name_length + 1);
	data_index += name_length;
	dave_strcpy(param->value, &data_ptr[data_index], value_length + 1);
	data_index += value_length;

	if(pFcgi->params == NULL)
	{
		pFcgi->params = param;
	}
	else
	{
		end_param = pFcgi->params;

		safe_counter = 0;

		while((end_param->next != NULL) && ((++ safe_counter) < 2048)) end_param = end_param->next;

		if(safe_counter >= 2048)
		{
			_http_fastcgi_free_param(param);
		}
		else
		{
			end_param->next = param;
		}
	}

	return data_index;
}

static void
_http_fastcgi_parse_param(u8 *data_ptr, ub data_len, Fcgi *pFcgi)
{
	ub data_index, safe_counter;

	if(data_len == 0)
	{
		HTTPDEBUG("param data-len is zero, the param end!");
		return;
	}

	data_index = safe_counter = 0;

	while((data_index < data_len) && ((++ safe_counter) < 4096))
	{
		data_index += _http_fastcgi_parse_param_(&data_ptr[data_index], data_len - data_index, pFcgi);
	}
}

static void
_http_fastcgi_parse_stdin(u8 *data_ptr, ub data_len, Fcgi *pFcgi)
{
	if(pFcgi->content_ptr == NULL)
	{
		pFcgi->content_length_max = FASTCGI_STDIN_MAX;
		pFcgi->content_ptr = dave_malloc(pFcgi->content_length_max);
		pFcgi->content_length = 0;
	}

	if((pFcgi->content_length + data_len + 1) <= pFcgi->content_length_max)
	{
		dave_memcpy(&(pFcgi->content_ptr[pFcgi->content_length]), data_ptr, data_len);
		pFcgi->content_length += data_len;
		pFcgi->content_ptr[pFcgi->content_length] = '\0';
	}
	else
	{
		HTTPABNOR("the http content too longer(%d)!", pFcgi->content_length + data_len);
	}
}

static ub
_http_fastcgi_parse(u8 *data_ptr, ub data_len, Fcgi *pFcgi, dave_bool pre_parse, dave_bool *parse_end, dave_bool *parse_error)
{
	ub head_len, process_len;

	*parse_end = dave_false;
	*parse_error = dave_false;

	*parse_error = _http_fastcgi_parse_head(data_ptr, data_len, &head_len, pFcgi);
	if(*parse_error == dave_true)
	{
		HTTPLOG("invalid head_len:%d data_len:%d pFcgi:%lx", head_len, data_len, pFcgi);
		return data_len;
	}

	process_len = head_len + pFcgi->head.content_length + pFcgi->head.padding_length;
	if(process_len > data_len)
	{
		HTTPDEBUG("wait more data, id:%lx process_len:%d head_len:%d content_len:%d padding_len:%d data_len:%d",
			pFcgi->head.request_id,
			process_len, head_len,
			pFcgi->head.content_length, pFcgi->head.padding_length,
			data_len);
		return data_len;
	}

	HTTPDEBUG("%d/%d type:%d content:%d padding:%d",
		head_len, data_len,
		pFcgi->head.type,
		pFcgi->head.content_length, pFcgi->head.padding_length);

	switch(pFcgi->head.type)
	{
		case FCGI_BEGIN_REQUEST:
				_http_fastcgi_parse_begin_request(&data_ptr[head_len], pFcgi->head.content_length, pFcgi);
			break;
		case FCGI_PARAMS:
				if(pre_parse == dave_false)
				{
					_http_fastcgi_parse_param(&data_ptr[head_len], pFcgi->head.content_length, pFcgi);
				}
			break;
		case FCGI_STDIN:
				if(pre_parse == dave_false)
				{
					if(pFcgi->head.content_length == 0)
					{
						*parse_end = dave_true;
					}
					else
					{
						_http_fastcgi_parse_stdin(&data_ptr[head_len], pFcgi->head.content_length, pFcgi);
					}
				}
				else
				{
					if(pFcgi->head.content_length == 0)
					{
						*parse_end = dave_true;
					}
				}
			break;
		default:
				HTTPABNOR("can't process type:%d", pFcgi->head.type);
				process_len = data_len;
			break;
	}

	return process_len;
}

static ub
_http_fastcgi_build_stdout(u8 *cgi_buf, ub cgi_len, ub request_id, s8 *content_ptr, ub content_len)
{
	ub cgi_index, padding_length;

	if(content_ptr == NULL)
	{
		content_len = 0;

		padding_length = 0;
	}
	else
	{
		padding_length = 8 - (content_len % 8);
	}

	cgi_index = 0;

	cgi_index += _http_fastcgi_build_head(&cgi_buf[cgi_index], cgi_len-cgi_index, FCGI_STDOUT, request_id, content_len, padding_length);
	if(content_len > 0)
	{
		if((content_len + padding_length) <= (cgi_len - cgi_index))
		{
			dave_memcpy(&cgi_buf[cgi_index], content_ptr, content_len);
			cgi_index += content_len;
			dave_memset(&cgi_buf[cgi_index], 0x00, padding_length);
			cgi_index += padding_length;
		}
		else
		{
			HTTPABNOR("invalid cgi_len:%d/%d/%d/%d", cgi_len, cgi_index, content_len, padding_length);
		}
	}

	return cgi_index;
}

static ub
_http_fastcgi_build_end_request(u8 *cgi_buf, ub cgi_len, ub request_id)
{
	FCGIEndRequestBody body;
	ub cgi_index;

	body.app_status = 0;
	body.protocol_status = FCGI_REQUEST_COMPLETE;
	body.reserved[0] = body.reserved[1] = body.reserved[2] = 0x00;

	cgi_index = 0;

	cgi_index += _http_fastcgi_build_head(&cgi_buf[cgi_index], cgi_len-cgi_index, FCGI_END_REQUEST, request_id, 8, 0);

	cgi_buf[cgi_index ++] = (u8)(body.app_status >> 24);
	cgi_buf[cgi_index ++] = (u8)(body.app_status >> 16);
	cgi_buf[cgi_index ++] = (u8)(body.app_status >> 8);
	cgi_buf[cgi_index ++] = (u8)(body.app_status);
	cgi_buf[cgi_index ++] = (u8)(body.protocol_status);
	cgi_buf[cgi_index ++] = body.reserved[0];
	cgi_buf[cgi_index ++] = body.reserved[1];
	cgi_buf[cgi_index ++] = body.reserved[2];

	return cgi_index;
}

static void
_http_fastcgi_add_head(ub *index, HttpKeyValue *pHead, s8 *key, s8 *value)
{
	if(*index >= DAVE_HTTP_HEAD_MAX)
	{
		HTTPABNOR("http head buffer is full!");
		return;
	}

	if((value == NULL) || (value[0] == '\0'))
	{
		return;
	}

	dave_strcpy(pHead[*index].key, key, DAVE_HTTP_KEY_LEN);
	dave_strcpy(pHead[*index].value, value, DAVE_HTTP_VALUE_LEN);

	(*index) ++;
}

static void
_http_fastcgi_load_head(HTTPRecvReq *pReq, ub *index, FCGIParamsBody *pParam, ub *content_length)
{
	HTTPDEBUG("Fcgi key:%s, value:%s", pParam->name, pParam->value);
	
	if(dave_strcmp(pParam->name, "REMOTE_ADDR") == dave_true)
	{
		dave_strcpy(pReq->remote_address, pParam->value, DAVE_URL_LEN);
	}
	else if(dave_strcmp(pParam->name, "REMOTE_PORT") == dave_true)
	{
		pReq->remote_port = stringdigital(pParam->value);
	}
	else if(dave_strcmp(pParam->name, "REQUEST_METHOD") == dave_true)
	{
		if(dave_strcmp(pParam->value, "POST") == dave_true)
		{
			pReq->method = HttpMethod_post;
		}
		else if(dave_strcmp(pParam->value, "GET") == dave_true)
		{
			pReq->method = HttpMethod_get;
		}
		else if(dave_strcmp(pParam->value, "PUT") == dave_true)
		{
			pReq->method = HttpMethod_put;
		}
		else if(dave_strcmp(pParam->value, "OPTIONS") == dave_true)
		{
			pReq->method = HttpMethod_options;
		}
		else if(dave_strcmp(pParam->value, "DELETE") == dave_true)
		{
			pReq->method = HttpMethod_delete;
		}
		else
		{
			HTTPLOG("What is this method:%s/%s?", pParam->name, pParam->value);
			pReq->method = HttpMethod_max;
		}
	}
	else if(dave_strcmp(pParam->name, "REQUEST_URI") == dave_true)
	{
		_http_fastcgi_add_head(index, pReq->head, "REQUEST_URI", pParam->value);
	}
	else if(dave_strcmp(pParam->name, "CONTENT_TYPE") == dave_true)
	{
		if(dave_strstr(pParam->value, "application/json") != NULL)
		{
			pReq->content_type = HttpContentType_json;
		}
		else if(dave_strcmp(pParam->value, "text/plain;charset=UTF-8") == dave_true)
		{
			pReq->content_type = HttpContentType_text;
		}
		else
		{
			pReq->content_type = HttpContentType_max;
		}
	}
	else if((dave_strcmp(pParam->name, "HTTP_AUTHORIZATION") == dave_true) 
		|| (dave_strcmp(pParam->name, "HTTPS_AUTHORIZATION") == dave_true))
	{
		_http_fastcgi_add_head(index, pReq->head, "Authorization", pParam->value);
	}
	else if((dave_strcmp(pParam->name, "HTTP_X_APP_BIZID") == dave_true) 
		|| (dave_strcmp(pParam->name, "HTTPS_X_APP_BIZID") == dave_true))
	{
		_http_fastcgi_add_head(index, pReq->head, "X-App-Bizid", pParam->value);
	}
	else if((dave_strcmp(pParam->name, "HTTP_X_APP_ID") == dave_true) 
		|| (dave_strcmp(pParam->name, "HTTPS_X_APP_ID") == dave_true))
	{
		_http_fastcgi_add_head(index, pReq->head, "X-App-Id", pParam->value);
	}
	else if((dave_strcmp(pParam->name, "HTTP_DIDI_HEADER_HINT_CONTENT") == dave_true) 
		|| (dave_strcmp(pParam->name, "HTTPS_DIDI_HEADER_HINT_CONTENT") == dave_true))
	{
		_http_fastcgi_add_head(index, pReq->head, "didi-header-hint-content", pParam->value);
	}
	else if(dave_strcmp(pParam->name, "CONTENT_LENGTH") == dave_true)
	{
		*content_length = stringdigital(pParam->value);
	}
	else if(dave_strcmp(pParam->name, "QUERY_STRING") == dave_true)
	{
		_http_fastcgi_add_head(index, pReq->head, "QUERY_STRING", pParam->value);
	}
}

// =====================================================================

dave_bool
http_fastcgi_parse(u8 *data_ptr, ub data_len, Fcgi *pFcgi, dave_bool pre_parse)
{
	ub data_index, safe_counter, parse_len;
	dave_bool parse_end, parse_error;

	if((data_ptr == NULL) || (data_len == 0))
	{
		HTTPABNOR("invalid data:%x or data_len:%d pFcgi:%lx", data_ptr, data_len, pFcgi);
		return dave_false;
	}

	parse_end = parse_error = dave_false;
	data_index = safe_counter = 0;

	while((parse_end == dave_false) && (data_index < data_len) && ((++ safe_counter) < 8192))
	{
		parse_len = _http_fastcgi_parse(&data_ptr[data_index], data_len-data_index, pFcgi, pre_parse, &parse_end, &parse_error);
		if(parse_error == dave_true)
		{
			HTTPLOG("%d/%d parse error! pFcgi:%lx", data_index, data_len, pFcgi);
			return dave_false;
		}

		data_index += parse_len;
	}
	if(safe_counter >= 8192)
	{
		HTTPABNOR("invalid safe_counter:%d", safe_counter);
	}

	if((parse_end == dave_true)
		&& (data_index != data_len))
	{
		HTTPLOG("lost data:%d/%d", data_index, data_len)
	}

	return parse_end;
}

void
http_fastcgi_parse_release(Fcgi *pFcgi)
{
	FCGIParamsBody *next_param;

	while(pFcgi->params != NULL)
	{
		next_param = pFcgi->params->next;

		_http_fastcgi_free_param(pFcgi->params);

		pFcgi->params = next_param;
	}

	pFcgi->params = NULL;

	if(pFcgi->content_ptr != NULL)
	{
		dave_free(pFcgi->content_ptr);
		pFcgi->content_ptr = NULL;
	}	
}

ub
http_fastcgi_build(Fcgi *pFcgi, HttpContentType content_type, s8 *content_ptr, ub content_len, u8 *cgi_ptr, ub cgi_len)
{
	s8 *type_str;
	ub cgi_index, content_index, stdout_len;

	cgi_index = 0;

	type_str = http_load_content_type(content_type);

	cgi_index += _http_fastcgi_build_stdout(&cgi_ptr[cgi_index], cgi_len-cgi_index, pFcgi->head.request_id, type_str, dave_strlen(type_str));
	content_index = 0;
	while(content_index < content_len)
	{
		if((content_len - content_index) > FASTCGI_STDOUT_MAX)
		{
			stdout_len = FASTCGI_STDOUT_MAX;
		}
		else
		{
			stdout_len = content_len - content_index;
		}

		cgi_index += _http_fastcgi_build_stdout(&cgi_ptr[cgi_index], cgi_len-cgi_index, pFcgi->head.request_id, &content_ptr[content_index], stdout_len);
		content_index += stdout_len;
	}
	cgi_index += _http_fastcgi_build_stdout(&cgi_ptr[cgi_index], cgi_len-cgi_index, pFcgi->head.request_id, NULL, 0);
	cgi_index += _http_fastcgi_build_end_request(&cgi_ptr[cgi_index], cgi_len-cgi_index, pFcgi->head.request_id);

	return cgi_index;
}

ub
http_fastcgi_build_error(Fcgi *pFcgi, u8 *cgi_buf, ub cgi_len)
{
	ub cgi_index;

	cgi_index = 0;

	cgi_index += _http_fastcgi_build_stdout(&cgi_buf[cgi_index], cgi_len-cgi_index, pFcgi->head.request_id, NULL, 0);
	cgi_index += _http_fastcgi_build_end_request(&cgi_buf[cgi_index], cgi_len-cgi_index, pFcgi->head.request_id);

	return cgi_index;
}

ub
http_fastcgi_build_ok(Fcgi *pFcgi, u8 *cgi_buf, ub cgi_len)
{
	s8 *type_str;
	ub cgi_index;

	cgi_index = 0;

	type_str = (s8 *)"Content-Length: 0\r\n\r\n";

	cgi_index += _http_fastcgi_build_stdout(&cgi_buf[cgi_index], cgi_len-cgi_index, pFcgi->head.request_id, type_str, dave_strlen(type_str));
	cgi_index += _http_fastcgi_build_stdout(&cgi_buf[cgi_index], cgi_len-cgi_index, pFcgi->head.request_id, NULL, 0);
	cgi_index += _http_fastcgi_build_end_request(&cgi_buf[cgi_index], cgi_len-cgi_index, pFcgi->head.request_id);

	return cgi_index;
}

ub
http_fastcgi_load_head(HTTPRecvReq *pReq, FCGIParamsBody *pParams)
{
	ub content_length = 0;
	ub index;

	for(index=0; index<DAVE_HTTP_HEAD_MAX; index++)
	{
		pReq->head[index].key[0] = '\0';
		pReq->head[index].value[0] = '\0';
	}

	pReq->remote_address[0] = '\0';
	pReq->remote_port = 0;
	pReq->method = HttpMethod_max;
	pReq->content_type = HttpContentType_max;

	index = 0;

	while(pParams != NULL)
	{
		HTTPDEBUG("%s:%s", pParams->name, pParams->value);

		_http_fastcgi_load_head(pReq, &index, pParams, &content_length);
		pParams = pParams->next;
	}

	if(pReq->method == HttpMethod_max)
	{
		HTTPABNOR("can't find method!");
	}

	return content_length;
}

FCGIParamsBody *
http_fastcgi_find_head(FCGIParamsBody *pParams, char *name)
{
	ub safe_counter;

	safe_counter = 0;

	while((pParams != NULL) && ((++ safe_counter) < 4096))
	{
		if(dave_strcmp(pParams->name, name) == dave_true)
		{
			return pParams;
		}

		pParams = pParams->next;
	}

	if(safe_counter >= 4096)
	{
		HTTPABNOR("find head error!");
	}

	return NULL;
}

