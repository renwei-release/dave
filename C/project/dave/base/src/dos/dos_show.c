/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "dos_show.h"
#include "dos_tty.h"
#include "dos_prompt.h"
#include "dos_log.h"

static void
_dos_show(s8 *msg)
{
	ub msg_len, prompt_len;
	s8 *data;
	ub data_index;

	msg_len = dave_strlen(msg);
	prompt_len = dave_strlen(dos_prompt());
	data = dave_malloc(msg_len + prompt_len + 64);
	if(data != NULL)
	{
		data_index = 0;
		dave_memcpy(&data[data_index], msg, msg_len);
		data_index += msg_len;
		data[data_index ++] = '\r';
		data[data_index ++] = '\n';
		data_index += dave_sprintf(&data[data_index], "%s", dos_prompt());
		data[data_index ++] = '\r';
		data[data_index ++] = '\n';
		data[data_index] = '\0';
		DOSLOG("%s", data);
		dos_tty_write((u8 *)data, data_index);
		dave_free(data);
	}
}

// =====================================================================

void
dos_show_init(void)
{

}

void
dos_show_exit(void)
{

}

void
dos_view(s8 *msg)
{
	ub msg_len;
	s8 *data;
	ub data_index;

	msg_len = dave_strlen(msg);
	data = dave_malloc(msg_len + 64);
	if(data != NULL)
	{
		data_index = 0;
		dave_memcpy(&data[data_index], msg, msg_len);
		data_index += msg_len;
		data[data_index ++] = '\r';
		data[data_index ++] = '\n';
		data[data_index ++] = '\0';
		dos_tty_write((u8 *)data, data_index);
		dave_free(data);
	}
}

void
dos_print(const char *fmt, ...)
{
	#define MAXSHOWBUF 16384
	va_list args;
	char *show_buf;

	show_buf = dave_malloc(MAXSHOWBUF);

	va_start(args, fmt);
	vsnprintf(show_buf, MAXSHOWBUF, fmt, args);
	va_end(args);

	_dos_show((s8 *)show_buf);

	dave_free(show_buf);
}

void
dos_show_prompt(void)
{
	ub prompt_len;
	s8 *data;
	ub data_index;

	prompt_len = dave_strlen(dos_prompt());
	data = dave_malloc(prompt_len + 64);
	if(data != NULL)
	{
		data_index = 0;
		data_index += dave_sprintf(&data[data_index], "%s", dos_prompt());
		data[data_index ++] = '\r';
		data[data_index ++] = '\n';
		data[data_index ++] = '\0';
		dos_tty_write((u8 *)data, data_index);
		dave_free(data);
	}
}

#endif

