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
#include "dave_os.h"
#include "dos_pop.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

#define TTY_BUF_MAX (4096)

static ThreadId _tty_my_self;
static u8 _tty_buf[TTY_BUF_MAX + 1];
static ub _tty_buf_write_index;

static RetCode
_tty_hardware_sync_notify(ub notify_id)
{
	MsgOSNotify *pNotify = thread_msg(pNotify);

	if(snd_from_msg(_tty_my_self, _tty_my_self, MSGID_OS_NOTIFY, sizeof(MsgOSNotify), pNotify) == dave_true)
		return RetCode_OK;
	else
		return RetCode_Send_msg_failed;
}

static void
_tty_read_process(s8 *input_ptr, ub input_len)
{
	input_ptr[input_len] = '\0';

	if(t_is_all_show_char_or_rn((u8 *)input_ptr, (sb)input_len) == dave_true)
	{
		if(dos_pop_analysis(input_ptr, input_len) == dave_false)
		{
			dos_cmd_analysis(input_ptr, input_len);
		}
	}
	else
	{
		dos_print("Character format is not supported!");
	}
}

static void
_tty_read(MSGBODY *thread_msg)
{
	ub read_buf_len, start_index, read_index;

	if(_tty_buf_write_index >= TTY_BUF_MAX)
	{
		_tty_buf_write_index = 0;
	}

	read_buf_len = dave_os_tty_read(&_tty_buf[_tty_buf_write_index], TTY_BUF_MAX - _tty_buf_write_index);
	if(read_buf_len == 0)
	{
		return;
	}

	_tty_buf_write_index += read_buf_len;

	_tty_buf[_tty_buf_write_index] = '\0';

	DOSTRACE("read data<%d>:%s", _tty_buf_write_index, _tty_buf);

	read_index = 0;
	// check data start flag.
	while(read_index < _tty_buf_write_index)
	{
		if((_tty_buf[read_index] == '\r') || (_tty_buf[read_index] == '\n'))
		{
			read_index ++;
		}
		else
		{
			break;
		}
	}
	start_index = read_index;
	// check data end flag, \r or \n or \r\n.
	while(read_index < _tty_buf_write_index)
	{
		if((_tty_buf[read_index] == '\r') || (_tty_buf[read_index] == '\n'))
		{
			if(((read_index + 1) < _tty_buf_write_index)
				&& ((_tty_buf[read_index + 1] == '\r') || (_tty_buf[read_index + 1] == '\n')))
			{
				read_index ++;
			}

			_tty_read_process((s8 *)(&_tty_buf[start_index]), read_index - start_index + 1);

			_tty_buf_write_index = 0;
			break;
		}
		read_index ++;
	}
}

// =====================================================================

void
dos_tty_init(void)
{
	_tty_my_self = get_self();

	_tty_buf_write_index = 0;

	reg_msg(MSGID_OS_NOTIFY, _tty_read);

	if(dave_os_tty_init(_tty_hardware_sync_notify) == dave_true)
	{
		DOSTRACE("tty_init success!");
	}
	else
	{
		DOSLOG("tty_init fail!");
	}
}

void
dos_tty_exit(void)
{
	dave_os_tty_exit();

	unreg_msg(MSGID_OS_NOTIFY);
}

void
dos_tty_write(u8 *msg_ptr, ub msg_len)
{
	dave_os_tty_write(msg_ptr, msg_len);
}

ub
dos_tty_read(s8 *msg_ptr, ub msg_len, ub wait_second)
{
	dave_memset(msg_ptr, 0x00, msg_len);

	msg_len = dave_os_tty_get((u8 *)msg_ptr, msg_len, wait_second);
	if(msg_len == 0)
	{
		dave_memset(msg_ptr, 0x00, msg_len);
		return 0;
	}

	if(msg_ptr[msg_len - 1] != '\n')
	{
		msg_ptr[0] = '\0';
		return 0;
	}
	else
	{
		msg_ptr[msg_len - 1] = '\0';
		msg_len -= 1;
	}

	return msg_len;
}

#endif

