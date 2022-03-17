/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "tools_log.h"

// =====================================================================

void
t_stdio_print_hex(const char *msg, u8 *hex, ub hex_len)
{
	ub buf_len;
	s8 *buf;
	ub buf_index, hex_index, tab_counter, add_len;

	buf_len = 64 + dave_strlen(msg) + hex_len*8 + 256;
	buf = dave_malloc(buf_len);
	buf_index = 0;
	hex_index = 0;
	tab_counter = 0;

	add_len = dave_snprintf(&buf[buf_index], buf_len-buf_index, "%s %d[", (s8 *)msg, hex_len);
	buf_index += add_len;
	tab_counter += add_len;
	while(hex_index < hex_len)
	{
		if((buf_index + 64) >= buf_len)
		{
			buf_index += dave_snprintf(&buf[buf_index], buf_len-buf_index, "*");
			break;
		}
		add_len = dave_snprintf(&buf[buf_index], buf_len-buf_index, "0x%02x, ", hex[hex_index]);
		buf_index += add_len;
		tab_counter += add_len;
		if(tab_counter > 120)
		{
			buf_index += dave_snprintf(&buf[buf_index], buf_len-buf_index, "\n");
			tab_counter = 0;
		}
		hex_index ++;
	}
	buf_index += dave_snprintf(&buf[buf_index], buf_len-buf_index, "]\n");

	TOOLSLOG("%s", buf);

	dave_free(buf);
}

void
t_stdio_print_char(const char *msg, u8 *char_data, ub char_len)
{
	ub buf_len;
	s8 *buf;
	ub buf_index, hex_index, tab_counter, add_len;

	buf_len = 64 + dave_strlen(msg) + char_len*8 + 256;
	buf = dave_malloc(buf_len);
	buf_index = 0;
	hex_index = 0;
	tab_counter = 0;

	add_len = dave_snprintf(&buf[buf_index], buf_len-buf_index, "%s %d<", (s8 *)msg, char_len);
	buf_index += add_len;
	tab_counter += add_len;
	while(hex_index < char_len)
	{
		if((buf_index + 64) >= buf_len)
		{
			buf_index += dave_snprintf(&buf[buf_index], buf_len-buf_index, "*");
			break;
		}
		if(t_is_show_char(char_data[hex_index]) == dave_true)
		{
			add_len = dave_snprintf(&buf[buf_index], buf_len-buf_index, "%c", char_data[hex_index]);
		}
		else
		{
			add_len = dave_snprintf(&buf[buf_index], buf_len-buf_index, "[%02x]", char_data[hex_index]);
		}
		buf_index += add_len;
		tab_counter += add_len;
		if(tab_counter > 120)
		{
			buf_index += dave_snprintf(&buf[buf_index], buf_len-buf_index, "\n");
			tab_counter = 0;
		}
		hex_index ++;
	}
	buf_index += dave_snprintf(&buf[buf_index], buf_len-buf_index, ">\n");

	TOOLSLOG("%s", buf);

	dave_free(buf);
}

