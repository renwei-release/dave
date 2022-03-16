/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_struct.h"
#include "thread_quit.h"
#include "thread_log.h"

#define QUIT_REASON_MESSAGE_LEN (256)

/*
 * 有些子线程任务退出时间会比较长，
 * 比如，
 * 退出的时候要做大量内存释放的工作等。
 */
#define WAIT_TIMER_MAX (5)

typedef struct {
	QUITTYPE type;
	s8 reason[QUIT_REASON_MESSAGE_LEN];
	TIMERID quit_timer;
	sb wait_timer;
	ThreadId who_have_restart_msg[THREAD_MAX];
	ThreadStruct *task;
	ub task_num;
} SYSTEMQUIT;

static SYSTEMQUIT _msg_quit;
static void *_system_wakeup_ptr = NULL;

static void
_system_quit(void)
{
	dave_os_thread_wakeup(_system_wakeup_ptr);

	switch(_msg_quit.type)
	{
		case QUIT_TYPE_RESTART:
		case QUIT_TYPE_POWER_OFF:
				dave_os_power_off(_msg_quit.reason);
			break;
		default:
				THREADLOG("system quit");
			break;
	}
}

static void
_clean_who_have_restart_msg(void)
{
	sw_ubase index;

	for(index=0; index<THREAD_MAX; index++)
	{
		_msg_quit.who_have_restart_msg[index] = INVALID_THREAD_ID;
	}
}

static void
_broadcast_system_quit_message(void)
{
	ub task_index;
	RESTARTREQMSG *pRestart;
	sb times;

	if(_msg_quit.task_num > THREAD_MAX)
	{
		_msg_quit.task_num = THREAD_MAX;
	}

	_clean_who_have_restart_msg();

	if(_msg_quit.wait_timer < 0)
	{
		times = 0;
	}
	else
	{
		times = WAIT_TIMER_MAX - _msg_quit.wait_timer;
		_msg_quit.wait_timer --;
	}

	for(task_index=0; task_index<_msg_quit.task_num; task_index++)
	{
		if((_msg_quit.task[task_index].thread_id != INVALID_THREAD_ID)
			&& (_msg_quit.task[task_index].attrib == LOCAL_TASK_ATTRIB))
		{
			_msg_quit.who_have_restart_msg[task_index] = _msg_quit.task[task_index].thread_id;

			pRestart = thread_msg(pRestart);
			dave_strcpy(pRestart->reason, _msg_quit.reason, sizeof(pRestart->reason));
			pRestart->times = times;

			write_msg(_msg_quit.task[task_index].thread_id, MSGID_RESTART_REQ, pRestart);
		}
	}
}

static void
_quit_timer_process(TIMERID timer_id, ub thread_index)
{
    if(_msg_quit.quit_timer != timer_id)
    {
        return;
    }

	if(_msg_quit.wait_timer <= 0)
	{
		_system_quit();
	}
	else
	{
		if(_msg_quit.wait_timer == 1)
		{
			if(_msg_quit.type == QUIT_TYPE_RESTART)
			{
				THREADLOG("System restart! (%s)", _msg_quit.reason);
			}
			else
			{
				THREADLOG("System power off! (%s)", _msg_quit.reason);
			}
		}
		else
		{
			if(_msg_quit.type == QUIT_TYPE_RESTART)
			{
				THREADLOG("System restart after %d seconds!", _msg_quit.wait_timer);
			}
			else
			{
				THREADLOG("System power off after %d seconds!", _msg_quit.wait_timer);
			}
		}

		_broadcast_system_quit_message();
	}
}

static void
_start_quit_timer(void)
{
	_msg_quit.quit_timer = base_timer_creat("QUIT", _quit_timer_process, 1000);

	if(_msg_quit.quit_timer == INVALID_TIMER_ID)
	{
		_system_quit();
	}
}

// =====================================================================

void
thread_quit_init(void *main_thread_id)
{
	_msg_quit.type = QUIT_TYPE_MAX;
	_system_wakeup_ptr = main_thread_id;
}

void
thread_quit_exit(void)
{

}

void
thread_quit(QUITTYPE type, s8 *reason, ThreadStruct *task, ub task_num)
{
	ub reason_len;
	
	if((type == QUIT_TYPE_RESTART) && (_msg_quit.type == QUIT_TYPE_MAX))
	{
		_msg_quit.type = type;
		dave_memset(_msg_quit.reason, 0x00, QUIT_REASON_MESSAGE_LEN);
		reason_len = dave_strlen(reason);
		if(reason_len > QUIT_REASON_MESSAGE_LEN-1)
			reason_len = QUIT_REASON_MESSAGE_LEN-1;
		dave_memcpy(_msg_quit.reason, reason, reason_len);
		_msg_quit.quit_timer = INVALID_TIMER_ID;
		_msg_quit.wait_timer = WAIT_TIMER_MAX;
		_msg_quit.task = task;
		_msg_quit.task_num = task_num;
		_start_quit_timer();
	}
}

#endif

