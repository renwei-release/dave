/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_os.h"
#include "uip_parsing.h"
#include "uip_tools.h"
#include "uip_log.h"

#define THE_MY_MAGIC_DATA 0x9988aaeec3

static inline void
_uip_decode_version(UIPStack *pStack, void *pJson)
{
	if(dave_json_get_ub(pJson, UIP_JSON_VERSION, &(pStack->version)) == dave_false)
	{
		UIPLOG("unknown version!");
		pStack->version = UIP_VERSION;
	}
}

static inline void
_uip_encode_version(void *pJson, UIPStack *pStack)
{
	dave_json_add_ub(pJson, UIP_JSON_VERSION, pStack->version);
}

static inline MBUF *
_uip_decode_customer_head(void *pJson)
{
	void *customer_head;

	customer_head = dave_json_get_object(pJson, UIP_JSON_CUSTOMER_HEAD);
	if(customer_head == NULL)
	{
		return NULL;
	}

	return dave_json_to_mbuf(customer_head);
}

static inline void *
_uip_encode_customer_head(MBUF *customer_head)
{
	if(customer_head == NULL)
	{
		return NULL;
	}

	return dave_string_to_json(customer_head->payload, customer_head->len);
}

static inline void
_uip_decode_head(UIPHead *pHead, void *pJsonHead)
{
	sb result_code;
	ub current_milliseconds;

	if(dave_json_get_sb(pJsonHead, UIP_JSON_RESULT_CODE, &result_code) == dave_true)
	{
		pHead->req_flag = dave_false;
		pHead->rsp_code = result_code;
	}
	else
	{
		pHead->req_flag = dave_true;
		pHead->rsp_code = RetCode_OK;
	}
	dave_json_get_str_v2(pJsonHead, UIP_JSON_METHOD, pHead->method, sizeof(pHead->method));
	dave_json_get_str_v2(pJsonHead, UIP_JSON_CHANNEL, pHead->channel, sizeof(pHead->channel));
	dave_json_get_str_v2(pJsonHead, UIP_JSON_AUTH_KEY, pHead->auth_key_str, sizeof(pHead->auth_key_str));
	if(dave_json_get_ub(pJsonHead, UIP_JSON_CURRENT_MILLISECONDS, &current_milliseconds) == dave_true)
	{
		pHead->current_milliseconds = current_milliseconds;
	}
	dave_json_get_ub(pJsonHead, UIP_JSON_SERIAL, &(pHead->serial));
	pHead->customer_head = _uip_decode_customer_head(pJsonHead);
}

static inline void *
_uip_encode_head(UIPHead *pHead)
{
	void *pJsonHead = dave_json_malloc();

	if(pHead->req_flag == dave_false)
	{
		dave_json_add_sb(pJsonHead, UIP_JSON_RESULT_CODE, uip_ret_to_code(pHead->rsp_code));
		dave_json_add_str(pJsonHead, UIP_JSON_RESULT_DESC, uip_ret_to_desc(pHead->rsp_code));
	}
	dave_json_add_str(pJsonHead, UIP_JSON_METHOD, pHead->method);
	dave_json_add_str(pJsonHead, UIP_JSON_CHANNEL, pHead->channel);
	if(pHead->req_flag == dave_true)
	{
		dave_json_add_str(pJsonHead, UIP_JSON_AUTH_KEY, pHead->auth_key_str);
	}
	dave_json_add_ub(pJsonHead, UIP_JSON_CURRENT_MILLISECONDS, pHead->current_milliseconds);
	dave_json_add_ub(pJsonHead, UIP_JSON_SERIAL, pHead->serial);
	if(pHead->customer_head != NULL)
	{
		dave_json_add_object(pJsonHead, UIP_JSON_CUSTOMER_HEAD, _uip_encode_customer_head(pHead->customer_head));
	}

	return pJsonHead;
}

static inline MBUF *
_uip_decode_customer_body(void *pJson)
{
	void *customer_body;

	customer_body = dave_json_get_object(pJson, UIP_JSON_CUSTOMER_BODY);
	if(customer_body == NULL)
	{
		return NULL;
	}

	return dave_json_to_mbuf(customer_body);
}

static inline void *
_uip_encode_customer_body(MBUF *customer_body)
{
	if(customer_body == NULL)
	{
		return NULL;
	}

	return dave_string_to_json(customer_body->payload, customer_body->len);
}

static inline void
_uip_decode_body(UIPBody *pBody, void *pJsonBody)
{
	pBody->pJson = pJsonBody;
	pBody->customer_body = _uip_decode_customer_body(pJsonBody);
}

static inline void *
_uip_encode_body(UIPBody *pBody)
{
	void *pJsonBody;

	if(pBody->pJson != NULL)
	{
		pJsonBody = dave_json_clone(pBody->pJson);
	}
	else
	{
		pJsonBody = dave_json_malloc();
		dave_json_add_str(pJsonBody, "EMPTY", (s8 *)"EMPTY");
	}
	if(pBody->customer_body != NULL)
	{
		dave_json_add_object(pJsonBody, UIP_JSON_CUSTOMER_BODY, _uip_encode_customer_body(pBody->customer_body));
	}

	return pJsonBody;
}

static inline UIPStack *
_uip_decode_to_stack(void *pJson)
{
	UIPStack *pStack = uip_malloc();
	void *pJsonHead, *pJsonBody;

	_uip_decode_version(pStack, pJson);

	pJsonHead = dave_json_get_object(pJson, UIP_JSON_HEAD);
	pJsonBody = dave_json_get_object(pJson, UIP_JSON_BODY);

	if(pJsonHead != NULL)
	{
		_uip_decode_head(&(pStack->head), pJsonHead);
		_uip_decode_body(&(pStack->body), pJsonBody);
	}
	else
	{
		pStack->head.method[0] = '\0';
	}

	return pStack;
}

static inline void *
_uip_encode_to_json(UIPStack *pStack, dave_bool encode_body)
{
	void *pJson;

	pJson = dave_json_malloc();

	_uip_encode_version(pJson, pStack);

	dave_json_add_object(pJson, UIP_JSON_HEAD, _uip_encode_head(&(pStack->head)));

	if(encode_body == dave_true)
	{
		dave_json_add_object(pJson, UIP_JSON_BODY, _uip_encode_body(&(pStack->body)));
	}

	return pJson;
}

// =====================================================================

UIPStack *
uip_malloc(void)
{
	UIPStack *pStack;

	pStack = dave_ralloc(sizeof(UIPStack));

	pStack->magic_data = THE_MY_MAGIC_DATA;

	pStack->version = UIP_VERSION;
	pStack->head.current_milliseconds = dave_os_time_ms();
	pStack->head.customer_head = NULL;

	pStack->body.pJson = NULL;
	pStack->body.customer_body = NULL;

	pStack->src = INVALID_THREAD_ID;
	pStack->ptr = NULL;

	pStack->auto_release_json = NULL;

	return pStack;
}

void
uip_free(UIPStack *pStack)
{
	if(pStack != NULL)
	{
		if(pStack->magic_data != THE_MY_MAGIC_DATA)
		{
			UIPABNOR("invalid magic data:%x", pStack->magic_data);
		}
		else
		{
			if(pStack->head.customer_head != NULL)
			{
				dave_mfree(pStack->head.customer_head);
				pStack->head.customer_head = NULL;
			}

			if(pStack->body.customer_body != NULL)
			{
				dave_mfree(pStack->body.customer_body);
				pStack->body.customer_body = NULL;
			}

			if(pStack->auto_release_json != NULL)
			{
				dave_json_free(pStack->auto_release_json);
				pStack->auto_release_json = NULL;
			}
		}

		dave_free(pStack);
	}
}

UIPStack *
uip_clone(UIPStack *pStack)
{
	UIPStack *pCloneStack;

	if(pStack == NULL)
	{
		return NULL;
	}

	pCloneStack = uip_malloc();

	*pCloneStack = *pStack;

	pCloneStack->head.current_milliseconds = dave_os_time_ms();
	if(pStack->head.customer_head != NULL)
	{
		pCloneStack->head.customer_head = dave_mclone(pStack->head.customer_head);
	}

	pCloneStack->body.pJson = NULL;

	if(pStack->body.customer_body != NULL)
	{
		pCloneStack->body.customer_body = dave_mclone(pStack->body.customer_body);
	}

	pCloneStack->auto_release_json = NULL;

	return pCloneStack;
}

UIPStack *
uip_decode(ThreadId src, void *ptr, void *pJson)
{
	UIPStack *pStack = _uip_decode_to_stack(pJson);

	pStack->src = src;
	pStack->ptr = ptr;

	pStack->auto_release_json = pJson;

	return pStack;
}

void *
uip_encode(UIPStack *pStack, dave_bool encode_body)
{
	return _uip_encode_to_json(pStack, encode_body);
}

ub
uip_encode_error(s8 *data_buf, ub data_length, RetCode ret)
{
	UIPStack error_stack;
	void *pJson;
	s8 *json_ptr;
	ub json_length;

	dave_memset(&error_stack, 0x00, sizeof(UIPStack));
	error_stack.head.req_flag = dave_false;
	error_stack.head.rsp_code = ret;

	pJson = _uip_encode_to_json(&error_stack, dave_true);

	json_ptr = dave_json_to_string(pJson, &json_length);

	if(json_length > (data_length - 1))
	{
		UIPABNOR("invalid json length:%d/%d", json_length, data_length);
		json_length = data_length - 1;
	}

	data_length = dave_memcpy(data_buf, json_ptr, json_length);
	data_buf[data_length] = '\0';

	dave_json_free(pJson);

	return data_length;
}

