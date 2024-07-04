/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_tools.h"
#include "rtc_param.h"
#include "rtc_log.h"

static ub
_tlv_parse_get_tlv(int *r_tag, int *r_length, unsigned char **r_value, s8 *buffer_ptr, ub buffer_len)
{
	int tag, length;

	if(buffer_len < 8)
		return 0;

	tag = (*(int *)(buffer_ptr));
	length = (*(int *)(buffer_ptr + 4));

	if((8 + length) > buffer_len)
		return 0;

	*r_tag = tag;
	*r_length = length;
	*r_value = (unsigned char *)(&buffer_ptr[8]);

	return 8 + length;
}

static ub
_tlv_parse_set_tlv(s8 *buffer_ptr, ub buffer_len, int tag, int length, unsigned char *value)
{
	if(buffer_len < (8 + length))
	{
		RTCLOG("buffer_len(%d) < (8 + length(%d))", buffer_len, length);
		return 0;
	}

	(*(int *)(buffer_ptr)) = tag;
	(*(int *)(buffer_ptr + 4)) = length;

	dave_memcpy(&buffer_ptr[8], value, length);

	return 8 + length;
}

static dave_bool
_tlv_parse_get_tag(int tag, s8 **value_ptr, ub *value_len, s8 *tlv_ptr, ub tlv_len)
{
	int t_tag, t_length;
	unsigned char *t_value;
	ub tlv_index, process_length;

	tlv_index = 0;

	while(tlv_index < tlv_len)
	{
		process_length = _tlv_parse_get_tlv(&t_tag, &t_length, &t_value, &tlv_ptr[tlv_index], tlv_len-tlv_index);
		if(process_length == 0)
			return dave_false;

		if(t_tag == tag)
		{
			*value_ptr = (s8 *)t_value;
			*value_len = (ub)t_length;
			return dave_true;
		}

		tlv_index += process_length;
	}

	return dave_false;
}

// =====================================================================

ub
tlv_parse_find_end(RTCClient *pClient)
{
	int tag, length;
	unsigned char *value;
	ub process_length;
	dave_bool find_flag = dave_false;

	while(pClient->tlv_buffer_r_index < pClient->tlv_buffer_w_index)
	{
		process_length = _tlv_parse_get_tlv(
			&tag, &length, &value,
			&pClient->tlv_buffer_ptr[pClient->tlv_buffer_r_index], pClient->tlv_buffer_w_index-pClient->tlv_buffer_r_index
		);

		if(process_length == 0)
			break;

		pClient->tlv_buffer_r_index += process_length;

		if(tag == TLV_TAG_END)
		{
			find_flag = dave_true;
			break;
		}
	}

	if(find_flag == dave_false)
		return 0;

	return pClient->tlv_buffer_r_index;
}

dave_bool
tlv_parse_get_token(s8 *token_ptr, ub token_len, s8 *tlv_ptr, ub tlv_len)
{
	s8 *value_ptr;
	ub value_len;

	if(_tlv_parse_get_tag(TLV_TAG_TOKEN, &value_ptr, &value_len, tlv_ptr, tlv_len) == dave_false)
	{
		RTCLOG("can't get token data");
		return dave_false;
	}

	dave_strcpy(token_ptr, value_ptr, token_len);

	return dave_true;
}

dave_bool
tlv_parse_get_id(s8 *id_ptr, ub id_len, s8 *tlv_ptr, ub tlv_len)
{
	s8 *value_ptr;
	ub value_len;

	if(_tlv_parse_get_tag(TLV_TAG_ID, &value_ptr, &value_len, tlv_ptr, tlv_len) == dave_false)
		return dave_false;

	dave_strcpy(id_ptr, value_ptr, id_len);

	return dave_true;
}

dave_bool
tlv_parse_get_app_data(s8 **value_ptr, ub *value_len, s8 *tlv_ptr, ub tlv_len)
{
	return _tlv_parse_get_tag(TLV_TAG_APP_DATA, value_ptr, value_len, tlv_ptr, tlv_len);
}

MBUF *
tlv_parse_set_app_data(s8 *token, s8 *data_ptr, ub data_len)
{
	MBUF *data = dave_mmalloc(1024 + data_len);
	ub tlv_index;
	s8 *tlv_ptr = ms8(data);
	ub tlv_len = mlen(data);

	tlv_index = 0;

	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_TOKEN, dave_strlen(token), (unsigned char *)token);
	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_APP_DATA, data_len, (unsigned char *)data_ptr);
	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_END, 3, (unsigned char *)"END");

	data->tot_len = data->len = tlv_index;

	return data;
}

MBUF *
tlv_parse_set_close(void)
{
	MBUF *data = dave_mmalloc(1024);
	ub tlv_index;
	s8 *tlv_ptr = ms8(data);
	ub tlv_len = mlen(data);

	tlv_index = 0;

	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_CLOSE, 5, (unsigned char *)"CLOSE");
	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_END, 3, (unsigned char *)"END");

	data->tot_len = data->len = tlv_index;

	return data;
}

