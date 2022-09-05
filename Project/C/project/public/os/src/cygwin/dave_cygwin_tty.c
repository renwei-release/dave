/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_CYGWIN__
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h> 
#include <netinet/in.h>
#include <time.h> 
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <termio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "os_log.h"

#define KEY_INPUT_MAX (2048)

static void *_linux_tty_thread = NULL;
static sync_notify_fun _notify_fun = NULL;
static s8 _keypad_char[KEY_INPUT_MAX];
static u32 _keypad_write = 0;
static u32 _keypad_read = 0;
static dave_bool _is_on_backend_printf_disable = dave_false;

static void *
_read_key_thread(void *arg)
{
	s8 keypad;

	_keypad_write = 0;
	_keypad_read = 0;

	struct termios attr;
    tcgetattr(STDIN_FILENO, &attr);
    attr.c_cc[VTIME] = 144;
    attr.c_cc[VMIN] = 0;
    attr.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &attr);

	while(dave_os_thread_canceled(_linux_tty_thread) == dave_false)
	{
		int ret = read(STDIN_FILENO, &keypad, 1);

		if(ret != 1)
		{
			continue;
		}

		_keypad_char[(_keypad_write ++) % KEY_INPUT_MAX] = keypad;

		if((keypad == '\n') && (_notify_fun != NULL))
		{
			_notify_fun(_keypad_write - _keypad_read);
		}
	}

	dave_os_thread_exit(_linux_tty_thread);

	_linux_tty_thread = NULL;

	return NULL;
}

// =====================================================================

dave_bool
dave_os_tty_init(sync_notify_fun notify_fun)
{
	_notify_fun = notify_fun;
	_keypad_write = 0;
	_keypad_read = 0;
	_is_on_backend_printf_disable = dave_false;

	_linux_tty_thread = dave_os_create_thread("tty", _read_key_thread, NULL);
	if(_linux_tty_thread == NULL)
	{
		OSABNOR("i can not start key thread!");
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

void
dave_os_tty_exit(void)
{
	_notify_fun = NULL;

	if(_linux_tty_thread != NULL)
	{
		dave_os_release_thread(_linux_tty_thread);
	}
}

void
dave_os_tty_write(u8 *data_ptr, ub data_len)
{
	if(_is_on_backend_printf_disable == dave_false)
	{
		if(fprintf(stdout, "\033[34m%s\033[0m", data_ptr) < 0)
		{
			_is_on_backend_printf_disable = dave_true;
		}
	}
}

ub
dave_os_tty_read(u8 *data_ptr, ub data_len)
{
	ub keypad_index;
	
	keypad_index = 0;
	
	while((keypad_index < data_len) && (_keypad_write > _keypad_read))
	{
		data_ptr[keypad_index ++] = (u8)((_keypad_char[(_keypad_read ++) % KEY_INPUT_MAX]));
	}

	return keypad_index;
}

void
dave_os_trace(TraceLevel level, u16 buf_len, u8 *buf_ptr)
{
	sb result;

	if(_is_on_backend_printf_disable == dave_false)
	{
		/*
		 * For definitions of color
		 * http://www.myjishu.com/?p=132
		 */

		if((level == TRACELEVEL_DEBUG) || (level == TRACELEVEL_CATCHER))
			result = fprintf(stdout, "\033[36m%s\033[0m", (char *)buf_ptr);
		else if(level == TRACELEVEL_TRACE)
			result = fprintf(stdout, "\033[33m%s\033[0m", (char *)buf_ptr);
		else if(level == TRACELEVEL_LOG)
			result = fprintf(stdout, "\033[38m%s\033[0m", (char *)buf_ptr);	
		else if(level == TRACELEVEL_ABNORMAL)
			result = fprintf(stdout, "\033[1m\033[35m%s\033[0m", (char *)buf_ptr);
		else if(level == TRACELEVEL_ASSERT)
			result = fprintf(stdout, "\033[1m\033[35m%s\033[0m", (char *)buf_ptr);
		else
			result = fprintf(stdout, "\033[28m%s\033[0m", (char *)buf_ptr);

		if(result < 0)
		{
			_is_on_backend_printf_disable = dave_true;
		}
	}
}

#endif

