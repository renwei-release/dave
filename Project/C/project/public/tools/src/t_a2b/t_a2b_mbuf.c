/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"

// =====================================================================

MBUF *
t_a2b_bin_to_mbuf(s8 *bin_ptr, ub bin_len)
{
	MBUF *mbuf_data;

	mbuf_data = dave_mmalloc(bin_len + 1);
	dave_memcpy(mbuf_data->payload, bin_ptr, bin_len);
	((s8 *)(mbuf_data->payload))[bin_len] = '\0';
	mbuf_data->len = mbuf_data->tot_len = bin_len;

	return mbuf_data;
}

MBUF *
t_a2b_str_to_mbuf(s8 *str_ptr, sb str_len)
{
	if(str_len <= 0)
	{
		str_len = dave_strlen(str_ptr);
	}

	return t_a2b_bin_to_mbuf(str_ptr, str_len);
}

MBUF *
t_a2b_param_to_mbuf(const char *args, ...)
{
	va_list list_args;
	int string_length = 1024 * 128;
	char *string_data;
	MBUF *mbuf_data;

	string_data = dave_malloc(string_length);

	va_start(list_args, args);
	string_length = vsnprintf(string_data, string_length, args, list_args);
	va_end(list_args);

	mbuf_data = t_a2b_str_to_mbuf((s8 *)string_data, (sb)string_length);

	dave_free(string_data);

	return mbuf_data;
}

ub
t_a2b_mbuf_to_buf(u8 *buf_ptr, ub buf_len, MBUF *m)
{
	ub buf_index;

	if((m == NULL) || (buf_ptr == NULL) || (buf_len == 0))
	{
		return 0;
	}

	buf_index = 0;

	while((m != NULL) && (buf_index < buf_len))
	{
		if((buf_index + m->len) <= buf_len)
		{
			dave_memcpy(&buf_ptr[buf_index], m->payload, m->len);
			buf_index += m->len;
			m = m->next;
		}
		else
		{
			dave_memcpy(&buf_ptr[buf_index], m->payload, buf_len-buf_index);
			buf_index = buf_len;
		}
	}

	return buf_index;
}

