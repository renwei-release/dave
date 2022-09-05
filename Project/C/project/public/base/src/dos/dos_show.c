/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
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
_dos_show(s8 *msg, dave_bool record)
{
	ub msg_len, prompt_len;
	s8 *show_ptr;
	ub show_index;

	msg_len = dave_strlen(msg);
	prompt_len = dave_strlen(dos_prompt());

	show_ptr = dave_malloc(msg_len + prompt_len + 64);

	show_index = 0;

	show_index += dave_memcpy(&show_ptr[show_index], msg, msg_len);
	show_ptr[show_index ++] = '\n';

	show_index += dave_sprintf(&show_ptr[show_index], "%s", dos_prompt());
	show_ptr[show_index ++] = '\n';
	show_ptr[show_index ++] = '\0';

	if(record == dave_true)
	{
		DOSLOG("%s", show_ptr);
	}

	dos_tty_write((u8 *)show_ptr, show_index);

	dave_free(show_ptr);
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
dos_print(const char *fmt, ...)
{
	#define MAXPRINTBUF 16384
	va_list args;
	char *show_buf;

	show_buf = dave_malloc(MAXPRINTBUF);

	va_start(args, fmt);
	vsnprintf(show_buf, MAXPRINTBUF, fmt, args);
	va_end(args);

	_dos_show(show_buf, dave_true);

	dave_free(show_buf);
}

void
dos_write(const char *fmt, ...)
{
	#define MAXWRITEBUF 65536
	va_list args;
	char *show_buf;

	show_buf = dave_malloc(MAXWRITEBUF);

	va_start(args, fmt);
	vsnprintf(show_buf, MAXWRITEBUF, fmt, args);
	va_end(args);

	_dos_show(show_buf, dave_false);

	dave_free(show_buf);
}

#endif

