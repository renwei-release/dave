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
t_a2b_str_to_mbuf(s8 *str)
{
	ub str_len;
	MBUF *mbuf_data;

	str_len = dave_strlen(str);
	mbuf_data = dave_mmalloc(str_len);
	dave_memcpy(mbuf_data->payload, str, str_len);

	return mbuf_data;
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
t_a2b_mbuf_to_buf(MBUF *m, u8 *buf, ub buf_len)
{
	ub buf_index;

	if((m == NULL) || (buf == NULL) || (buf_len == 0))
	{
		return 0;
	}

	buf_index = 0;

	while((m != NULL) && (buf_index < buf_len))
	{
		if((buf_index + m->len) <= buf_len)
		{
			dave_memcpy(&buf[buf_index], m->payload, m->len);
			buf_index += m->len;
			m = m->next;
		}
		else
		{
			dave_memcpy(&buf[buf_index], m->payload, buf_len-buf_index);
			buf_index = buf_len;
		}
	}

	return buf_index;
}

ub 
t_a2b_mbufs_to_mbuf(MBUF **dst, MBUF *src)
{
	ub n_len, index=0;
	MBUF *n = NULL;
	MBUF *s = src;
	
	if((dst == NULL) || (src == NULL))
	{
		return 0;
	}

	*dst = NULL;
	n_len = src->tot_len + 1;

	n = dave_mmalloc(n_len);
	if (NULL == n)
	{
		return 0;
	}

	while((src != NULL) && (index < n_len))
	{
		if((index + src->len) <= n_len)
		{
			dave_memcpy(&(((s8 *)(n->payload))[index]), src->payload, src->len);
			index += src->len;
			src = src->next;
		}
		else
		{
			dave_memcpy(&(((s8 *)(n->payload))[index]), src->payload, n_len-index);
			index = n_len;
		}
	}

	n->len = n->tot_len = index;
	*dst = n;
	
	dave_mfree(s);

	return index;
}

