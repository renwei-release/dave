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
#include "dave_verno.h"
#include "dos_cmd.h"
#include "dos_pop.h"
#include "dos_show.h"
#include "dos_tty.h"
#include "dos_app.h"
#include "dos_welcome.h"
#include "dos_log.h"

#define CFG_DOS_IS_PRIVATE "DOSIsPrivate"

static ThreadId _dos_thread = INVALID_THREAD_ID;

static ub
_base_dos_is_private(void)
{
	if(cfg_get_bool(CFG_DOS_IS_PRIVATE, dave_true) == dave_true)
	{
		return THREAD_PRIVATE_FLAG;
	}
	else
	{
		return 0x00;
	}
}

static s8 *
_base_dos_thread_name(s8 *thread_name, ub thread_len)
{
	s8 product_name[128];

	if(_base_dos_is_private() == dave_false)
	{
		dave_strcpy(product_name, dave_verno_my_product(), sizeof(product_name));
		lower(product_name);
		dave_snprintf(thread_name, thread_len, "%s_%s", DOS_THREAD_NAME, product_name);
	}
	else
	{
		dave_snprintf(thread_name, thread_len, "%s", DOS_THREAD_NAME);
	}

	return thread_name;
}

static void
_base_dos_restart(RESTARTREQMSG *pRestart)
{
	if(pRestart->times == 1)
	{
		dos_cmd_exit();
	}
}

static void
_base_dos_init(MSGBODY *task_msg)
{
	dos_tty_init();
	dos_pop_init();
	dos_show_init();
	dos_app_reset();
	dos_welcome_screen();
}

static void
_base_dos_main(MSGBODY *msg)
{
	switch((ub)msg->msg_id)
	{
		case MSGID_RESTART_REQ:
				_base_dos_restart((RESTARTREQMSG *)(msg->msg_body));
			break;
		default:
            DOSDEBUG("unprocess %s/%lx->%s/%lx:%d",
                thread_name(msg->msg_src), msg->msg_src,
                thread_name(msg->msg_dst), msg->msg_dst,
                msg->msg_id);
			break;
	}
}

static void
_base_dos_exit(MSGBODY *task_msg)
{
	dos_pop_exit();
	dos_show_exit();
	dos_tty_exit();
}

// =====================================================================

void
base_dos_init(void)
{
	ub thread_flag = THREAD_THREAD_FLAG|_base_dos_is_private();
	s8 thread_name[128];

	_base_dos_thread_name(thread_name, sizeof(thread_name));

	dos_cmd_init();

	_dos_thread = base_thread_creat(thread_name, 1, thread_flag, _base_dos_init, _base_dos_main, _base_dos_exit);
	if(_dos_thread == INVALID_THREAD_ID)
		base_restart(thread_name);
}

void
base_dos_exit(void)
{
	dos_cmd_exit();

	if(_dos_thread != INVALID_THREAD_ID)
		base_thread_del(_dos_thread);
	_dos_thread = INVALID_THREAD_ID;
}

#endif

