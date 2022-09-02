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

	mbuf_data = dave_mmalloc(bin_len);
	dave_memcpy(dave_mptr(mbuf_data), bin_ptr, bin_len);

	return mbuf_data;
}

MBUF *
t_a2b_str_to_mbuf(s8 *str)
{
	return t_a2b_bin_to_mbuf(str, dave_strlen(str));
}

MBUF *
t_a2b_param_to_mbuf(const char *args, ...)
{
	va_list list_args;
	char string_data[8196];

	va_start(list_args, args);
	vsnprintf(string_data, sizeof(string_data), args, list_args);
	va_end(list_args);

	return t_a2b_str_to_mbuf((s8 *)string_data);
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

