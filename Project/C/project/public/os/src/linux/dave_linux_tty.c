/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_LINUX__
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
#define KEY_WAIT_TIME (4)

static void *_tty_read_thread_body = NULL;
static sync_notify_fun _notify_fun = NULL;
static s8 _keypad_char[KEY_INPUT_MAX];
static ub _keypad_write = 0;
static ub _keypad_read = 0;
static TLock _keypad_pv; 
static dave_bool _is_on_backend_printf_disable = dave_false;

static void
_tty_trace_fprintf(TraceLevel level, u16 buf_len, s8 *buf_ptr)
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
		else if(level == TRACELEVEL_UI)
			result = fprintf(stdout, "\033[34m%s\033[0m", (char *)buf_ptr);
		else
			result = fprintf(stdout, "\033[28m%s\033[0m", (char *)buf_ptr);

		if(result < 0)
		{
			_is_on_backend_printf_disable = dave_true;
		}
	}
}

static void
_tty_trace_fput(TraceLevel level, u16 buf_len, s8 *buf_ptr)
{
	int fput_len = 128 + buf_len;
	char *fput_ptr = dave_malloc(fput_len);
	sb result;

	if(_is_on_backend_printf_disable == dave_false)
	{
		/*
		 * For definitions of color
		 * http://www.myjishu.com/?p=132
		 */

		if((level == TRACELEVEL_DEBUG) || (level == TRACELEVEL_CATCHER))
			dave_snprintf(fput_ptr, fput_len, "\033[36m%s\033[0m", (char *)buf_ptr);
		else if(level == TRACELEVEL_TRACE)
			dave_snprintf(fput_ptr, fput_len, "\033[33m%s\033[0m", (char *)buf_ptr);
		else if(level == TRACELEVEL_LOG)
			dave_snprintf(fput_ptr, fput_len, "\033[38m%s\033[0m", (char *)buf_ptr);	
		else if(level == TRACELEVEL_ABNORMAL)
			dave_snprintf(fput_ptr, fput_len, "\033[1m\033[35m%s\033[0m", (char *)buf_ptr);
		else if(level == TRACELEVEL_ASSERT)
			dave_snprintf(fput_ptr, fput_len, "\033[1m\033[35m%s\033[0m", (char *)buf_ptr);
		else if(level == TRACELEVEL_UI)
			dave_snprintf(fput_ptr, fput_len, "\033[34m%s\033[0m", (char *)buf_ptr);
		else
			dave_snprintf(fput_ptr, fput_len, "\033[28m%s\033[0m", (char *)buf_ptr);

		result = fputs(fput_ptr, stdout);

		if(result < 0)
		{
			_is_on_backend_printf_disable = dave_true;
		}
	}

	dave_free(fput_ptr);
}

static void
_tty_trace(TraceLevel level, u16 buf_len, s8 *buf_ptr)
{
	if(buf_len < 4095)
	{
		_tty_trace_fprintf(level, buf_len, buf_ptr);
	}
	else
	{
		_tty_trace_fput(level, buf_len, buf_ptr);
	}
}

static dave_bool
_tty_read_data(void)
{
	s8 keypad;

	int ret = read(STDIN_FILENO, &keypad, 1);

	if(ret != 1)
	{
		if ((errno == 0) || (errno == EINTR))
		{
			// normal read timeout or receive break signal
			return dave_true;
		}
		else
		{
			_is_on_backend_printf_disable = dave_true;
			return dave_false;
		}
	}

	SAFECODEv1(_keypad_pv, {
		_keypad_char[(_keypad_write ++) % KEY_INPUT_MAX] = keypad;
	});

	if((keypad == '\n') && (_notify_fun != NULL))
	{
		_notify_fun(_keypad_write - _keypad_read);
	}

	return dave_true;
}

static void *
_tty_read_thread(void *arg)
{
	SAFECODEv1(_keypad_pv, {
		_keypad_write = 0;
		_keypad_read = 0;
	});

	struct termios attr;
    tcgetattr(STDIN_FILENO, &attr);
    attr.c_cc[VTIME] = 10 * KEY_WAIT_TIME;
    attr.c_cc[VMIN] = 0;
    attr.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &attr);

	while(dave_os_thread_canceled(_tty_read_thread_body) == dave_false)
	{
		if(_tty_read_data() == dave_false)
			break;
	}

	return NULL;
}

static inline void
_tty_pre_init(void)
{
	static volatile sb __safe_pre_flag__ = 0;
	dave_bool thread_init = dave_false;

	SAFEPre(__safe_pre_flag__, {
		_notify_fun = NULL;
		_keypad_write = _keypad_read = 0;
		t_lock_reset(&_keypad_pv);
		_is_on_backend_printf_disable = dave_false;
		thread_init = dave_true;
	} );


	if(thread_init == dave_true)
	{
		_tty_read_thread_body = dave_os_create_thread("tty-read", _tty_read_thread, NULL);
		if(_tty_read_thread_body == NULL)
		{
			OSABNOR("i can not start tty read thread!");
		}
	}
}

// =====================================================================

dave_bool
dave_os_tty_init(sync_notify_fun notify_fun)
{
	_tty_pre_init();

	_notify_fun = notify_fun;

	return dave_true;
}

void
dave_os_tty_exit(void)
{
	_notify_fun = NULL;

	if(_tty_read_thread_body != NULL)
	{
		dave_os_release_thread(_tty_read_thread_body);
	}

	dave_os_sleep((KEY_WAIT_TIME + 1) * 1000);
}

void
dave_os_tty_write(u8 *data_ptr, ub data_len)
{
	dave_os_trace(TRACELEVEL_UI, data_len, (s8 *)data_ptr);
}

ub
dave_os_tty_read(u8 *data_ptr, ub data_len)
{
	ub keypad_index;
	
	keypad_index = 0;

	SAFECODEv1(_keypad_pv, {
		while((keypad_index < data_len) && (_keypad_write > _keypad_read))
		{
			data_ptr[keypad_index ++] = (u8)((_keypad_char[(_keypad_read ++) % KEY_INPUT_MAX]));
		}
	});

	return keypad_index;
}

ub
dave_os_tty_get(u8 *data_ptr, ub data_len, ub wait_second)
{
	ub sleep_base_millisecond = 100;
	ub wait_index, wait_times = (wait_second * 1000) / sleep_base_millisecond;
	sync_notify_fun fun;
	ub data_index;

	fun = _notify_fun;
	_notify_fun = NULL;

	data_index = wait_index = 0;

	while((data_len > data_index) && (wait_index < wait_times))
	{
		data_index += dave_os_tty_read(&data_ptr[data_index], data_len-data_index);
		if((data_index > 0) && (data_ptr[data_index - 1] == '\n'))
			break;

		dave_os_sleep(sleep_base_millisecond); wait_index ++;
	}

	_notify_fun = fun;

	return data_index;
}

void
dave_os_trace(TraceLevel level, ub data_len, s8 *data_ptr)
{
	_tty_trace(level, data_len, data_ptr);
}

#endif

