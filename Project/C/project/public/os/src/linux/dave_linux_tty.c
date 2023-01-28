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

typedef struct {
	TraceLevel level;
	ub data_len;
	u8 *data_ptr;

	void *next;
} TTYWriteChain;

static void *_tty_read_thread_body = NULL;
static void *_tty_write_thread_body = NULL;
static sync_notify_fun _notify_fun = NULL;
static s8 _keypad_char[KEY_INPUT_MAX];
static u32 _keypad_write = 0;
static u32 _keypad_read = 0;
static TLock _write_pv;
static TTYWriteChain *_write_chain_head = NULL;
static TTYWriteChain *_write_chain_tail = NULL;
static dave_bool _is_on_backend_printf_disable = dave_false;

static void
_tty_trace(TraceLevel level, u16 buf_len, u8 *buf_ptr)
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

static TTYWriteChain *
_tty_malloc_chain(TraceLevel level, ub data_len, u8 *data_ptr)
{
	TTYWriteChain *pChain = dave_malloc(sizeof(TTYWriteChain));

	pChain->level = level;
	pChain->data_len = data_len;
	pChain->data_ptr = dave_malloc(pChain->data_len + 1);
	dave_memcpy(pChain->data_ptr, data_ptr, data_len);
	pChain->data_ptr[data_len] = '\0';
	pChain->next = NULL;

	return pChain;
}

static void
_tty_free_chain(TTYWriteChain *pChain)
{
	if(pChain != NULL)
	{
		if(pChain->data_ptr != NULL)
			dave_free(pChain->data_ptr);

		dave_free(pChain);
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
	
	_keypad_char[(_keypad_write ++) % KEY_INPUT_MAX] = keypad;
	
	if((keypad == '\n') && (_notify_fun != NULL))
	{
		_notify_fun(_keypad_write - _keypad_read);
	}

	return dave_true;
}

static void
_tty_write_data(void)
{
	ub safe_counter;
	TTYWriteChain *pChain;

	safe_counter = 0;

	while((safe_counter ++) < 102400)
	{
		pChain = NULL;
	
		SAFECODEv1(_write_pv, {

			if(_write_chain_head != NULL)
			{
				pChain = _write_chain_head;

				_write_chain_head = _write_chain_head->next;

				if(_write_chain_head == NULL)
				{
					_write_chain_tail = NULL;
				}
			}

		});

		if(pChain == NULL)
		{
			break;
		}

		_tty_trace(pChain->level, pChain->data_len, pChain->data_ptr);

		_tty_free_chain(pChain);
	}
}

static void *
_tty_read_thread(void *arg)
{
	_keypad_write = 0;
	_keypad_read = 0;

	struct termios attr;
    tcgetattr(STDIN_FILENO, &attr);
    attr.c_cc[VTIME] = 40;
    attr.c_cc[VMIN] = 0;
    attr.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &attr);

	while(dave_os_thread_canceled(_tty_read_thread_body) == dave_false)
	{
		if(_tty_read_data() == dave_false)
			break;

		_tty_write_data();
	}

	dave_os_thread_exit(_tty_read_thread_body);

	_tty_read_thread_body = NULL;

	return NULL;
}

static void *
_tty_write_thread(void *arg)
{
	while(dave_os_thread_canceled(_tty_write_thread_body) == dave_false)
	{
		dave_os_thread_sleep(_tty_write_thread_body);
	
		_tty_write_data();
	}

	dave_os_thread_exit(_tty_write_thread_body);

	_tty_write_thread_body = NULL;

	_tty_write_data();

	return NULL;
}

// =====================================================================

dave_bool
dave_os_tty_init(sync_notify_fun notify_fun)
{
	_notify_fun = notify_fun;
	_keypad_write = _keypad_read = 0;
	t_lock_reset(&_write_pv);
	_write_chain_head = _write_chain_tail = NULL;
	_is_on_backend_printf_disable = dave_false;

	_tty_read_thread_body = dave_os_create_thread("tty-read", _tty_read_thread, NULL);
	if(_tty_read_thread_body == NULL)
	{
		OSABNOR("i can not start tty read thread!");
		return dave_false;
	}

	_tty_write_thread_body = dave_os_create_thread("tty-write", _tty_write_thread, NULL);
	if(_tty_read_thread_body == NULL)
	{
		OSABNOR("i can not start tty write thread!");
		return dave_false;
	}

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
	if(_tty_write_thread_body != NULL)
	{
		dave_os_release_thread(_tty_write_thread_body);
	}
}

void
dave_os_tty_write(u8 *data_ptr, ub data_len)
{
	dave_os_trace(TRACELEVEL_UI, data_len, data_ptr);
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
dave_os_trace(TraceLevel level, ub data_len, u8 *data_ptr)
{
	TTYWriteChain *pChain = _tty_malloc_chain(level, data_len, data_ptr);

	SAFECODEv1(_write_pv, {

		if(_write_chain_head == NULL)
		{
			_write_chain_head = _write_chain_tail = pChain;
		}
		else
		{
			_write_chain_tail->next = pChain;

			_write_chain_tail = pChain;
		}

	});

	dave_os_thread_wakeup(_tty_write_thread_body);
}

#endif

