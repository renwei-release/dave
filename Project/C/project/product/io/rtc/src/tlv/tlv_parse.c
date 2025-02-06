/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_tools.h"
#include "rtc_param.h"
#include "tlv_parse.h"
#include "rtc_log.h"

static ub
_tlv_parse_get_tlv(int *r_tag, int *r_length, unsigned char **r_value, s8 *buffer_ptr, ub buffer_len)
{
	int tag, length;

	if(buffer_len < 8)
		return 0;

	tag = ((int)buffer_ptr[0] << 24) | ((int)buffer_ptr[1] << 16) | ((int)buffer_ptr[2] << 8) | ((int)buffer_ptr[3]);
	length = ((int)buffer_ptr[4] << 24) | ((int)buffer_ptr[5] << 16) | ((int)buffer_ptr[6] << 8) | ((int)buffer_ptr[7]);

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

	buffer_ptr[0] = (s8)((tag >> 24) & 0xff);
	buffer_ptr[1] = (s8)((tag >> 16) & 0xff);
	buffer_ptr[2] = (s8)((tag >> 8) & 0xff);
	buffer_ptr[3] = (s8)(tag & 0xff);
	buffer_ptr[4] = (s8)((length >> 24) & 0xff);
	buffer_ptr[5] = (s8)((length >> 16) & 0xff);
	buffer_ptr[6] = (s8)((length >> 8) & 0xff);
	buffer_ptr[7] = (s8)(length & 0xff);

	dave_memcpy(&buffer_ptr[8], value, length);

	return 8 + length;
}

static u16
_tlv_parse_get_u16(s8 *tlv_ptr, ub tlv_len)
{
	u16 data_value;

	if(tlv_len < 2)
		return 0;

	data_value = ((u16)tlv_ptr[0] << 8) | ((u16)tlv_ptr[1]);

	return data_value;
}

static ub
_tlv_parse_set_u16(s8 *tlv_ptr, ub tlv_len, u16 data_value)
{
	if(tlv_len < 2)
	{
		RTCLOG("tlv_len(%d) < 2", tlv_len);
		return 0;
	}

	tlv_ptr[0] = (s8)((data_value >> 8) & 0xff);
	tlv_ptr[1] = (s8)(data_value & 0xff);

	return 2;
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

void
tlv_parse_get_tlv(RTCToken *pToken, u16 *my_data_serial, unsigned char **my_data_ptr, ub *my_data_length, s8 *tlv_ptr, ub tlv_len)
{
	int t_tag, t_length;
	unsigned char *t_value;
	ub tlv_index, process_length;

	tlv_index = 0;

	while(tlv_index < tlv_len)
	{
		process_length = _tlv_parse_get_tlv(&t_tag, &t_length, &t_value, &tlv_ptr[tlv_index], tlv_len-tlv_index);
		if(process_length == 0)
			return;
		tlv_index += process_length;

		switch(t_tag)
		{
			case TLV_TAG_START:
			case TLV_TAG_END:
				break;
			case TLV_TAG_TOKEN:
				break;
			case TLV_TAG_TERMINAL_TYPE:
					if(pToken->terminal_type[0] == '\0')
					{
						if(t_length >= sizeof(pToken->terminal_type))
						{
							RTCLOG("TLV_TAG_TERMINAL_TYPE:%d/%s is too long!", t_length, t_value);
							t_length = sizeof(pToken->terminal_type) - 1;
						}
						dave_memcpy(pToken->terminal_type, t_value, t_length);
					}
				break;
			case TLV_TAG_TERMINAL_ID:
					if(pToken->terminal_id[0] == '\0')
					{
						if(t_length >= sizeof(pToken->terminal_id))
						{
							RTCLOG("TLV_TAG_TERMINAL_ID:%d/%s is too long!", t_length, t_value);
							t_length = sizeof(pToken->terminal_id) - 1;
						}
						dave_memcpy(pToken->terminal_id, t_value, t_length);
					}
				break;
			case TLV_TAG_DATA_FORMAT:
					if(pToken->data_format[0] == '\0')
					{
						if(t_length >= sizeof(pToken->data_format))
						{
							RTCLOG("TLV_TAG_DATA_FORMAT:%d/%s is too long!", t_length, t_value);
							t_length = sizeof(pToken->data_format) - 1;
						}
						dave_memcpy(pToken->data_format, t_value, t_length);
					}
				break;
			case TLV_TAG_DATA_SERIAL:
					*my_data_serial = _tlv_parse_get_u16((s8 *)t_value, t_length);
				break;
			case TLV_TAG_DATA_BODY:
					*my_data_ptr = t_value;
					*my_data_length = t_length;
				break;
			default:
					RTCLOG("unprocess t_tag:%lx", t_tag);
				break;
		}
	}
}

MBUF *
tlv_parse_set_data(s8 *token, s8 *format, u16 serial, s8 *data_ptr, ub data_len)
{
	MBUF *data = dave_mmalloc(1024 + data_len);
	ub tlv_index;
	s8 *tlv_ptr = ms8(data);
	ub tlv_len = mlen(data);
	s8 serial_tlv_buffer[2];

	_tlv_parse_set_u16(serial_tlv_buffer, sizeof(serial_tlv_buffer), serial);

	tlv_index = 0;

	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_START, 3, (unsigned char *)"STA");
	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_TOKEN, dave_strlen(token), (unsigned char *)token);
	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_DATA_FORMAT, dave_strlen(format), (unsigned char *)format);
	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_DATA_SERIAL, sizeof(serial_tlv_buffer), (unsigned char *)serial_tlv_buffer);
	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_DATA_BODY, data_len, (unsigned char *)data_ptr);
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

	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_START, 3, (unsigned char *)"STA");
	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_DATA_CLOSE, 5, (unsigned char *)"CLOSE");
	tlv_index += _tlv_parse_set_tlv(&tlv_ptr[tlv_index], tlv_len-tlv_index, TLV_TAG_END, 3, (unsigned char *)"END");

	data->tot_len = data->len = tlv_index;

	return data;
}

