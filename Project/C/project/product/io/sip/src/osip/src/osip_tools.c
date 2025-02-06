/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "osip_param.h"
#include "osip_sdp.h"
#include "osip_log.h"


// =====================================================================

ub
osip_find_end_flag(s8 *data_ptr, ub data_len)
{
	ub data_index;

	data_index = 0;

	while((data_index + 4) <= data_len)
	{	
		if((data_ptr[data_index] == '\r')
			&& (data_ptr[data_index + 1] == '\n')
			&& (data_ptr[data_index + 2] == '\r')
			&& (data_ptr[data_index + 3] == '\n'))
		{
			return data_index + 4;
		}

		data_index += 1;
	}

	return 0;
}

ub
osip_content_length(s8 *data_ptr, ub data_len)
{
	ub data_index;

	data_index = 0;

	while(data_index < data_len)
	{
		if(data_ptr[data_index] == 'C')
		{
			if(dave_memcmp(&data_ptr[data_index], "Content-Length:", 15) == dave_true)
			{
				data_index += 15;
				break;
			}
		}
		data_index ++;
	}
	if(data_index >= data_len)
	{
		return 0;
	}

	while(data_index < data_len)
	{
		if((data_ptr[data_index] >= '0') && (data_ptr[data_index] <= '9'))
		{
			break;
		}
		data_index ++;
	}
	if(data_index >= data_len)
	{
		return 0;
	}

	return stringdigital(&data_ptr[data_index]);
}

s8 *
osip_get_from_user(osip_message_t *sip)
{
	if(sip == NULL)
	{
		OSIPLOG("sip is NULL!");
		return NULL;
	}

	if(sip->from == NULL)
	{
		OSIPLOG("from is NULL!");
		return NULL;
	}

	return (s8 *)osip_uri_get_username(osip_from_get_url(sip->from));
}

s8 *
osip_get_to_user(osip_message_t *sip)
{
	if(sip == NULL)
	{
		OSIPLOG("sip is NULL!");
		return NULL;
	}

	if(sip->to == NULL)
	{
		OSIPLOG("to is NULL!");
		return NULL;
	}

	return (s8 *)osip_uri_get_username(osip_from_get_url(sip->to));
}

s8 *
osip_get_call_id(osip_message_t *sip)
{
	if(sip == NULL)
	{
		OSIPLOG("sip is NULL!");
		return NULL;
	}

	if(sip->call_id == NULL)
	{
		OSIPLOG("call id is NULL!");
		return NULL;
	}

	return (s8 *)osip_call_id_get_number(sip->call_id);
}

s8 *
osip_get_cseq_method(osip_message_t *sip)
{
	if(sip == NULL)
		return NULL;

	if(sip->cseq == NULL)
		return NULL;

	return sip->cseq->method;
}

s8 *
osip_get_cseq_number(osip_message_t *sip)
{
	if(sip == NULL)
		return NULL;

	if(sip->cseq == NULL)
		return NULL;

	return sip->cseq->number;
}

