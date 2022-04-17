/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "timer_log.h"

#define TIMER_MAX (8192)
#define TIMER_NAME_LEN (16)
#define CREAT_SW_TIMER_MIN_VALUE (1000)						// ms
#define CREAT_SW_TIMER_MAX_VALUE (1000*60*60*24*7UL)		// ms

#define TIMER_THREAD_NAME "timer"

typedef enum {
	CREATE_TIMER,
	DIE_TIMER,
	STOP_TIMER,

	OP_TIMER_NULL
} TimerOPType;

typedef enum {
	HW_TIMER,
	SW_TIMER,
	NONE_TIMER
} TimerAttrib;

typedef struct {
	TIMERID timer_id;

	TLock opt_pv;
	TLock run_pv;

	base_timer_fun fun;
	base_timer_param_fun param_fun;
	void *param;
	ThreadId owner;

	ub alarm_ms;
	ub life_ms;
	ub time_out_counter;

	s8 name[TIMER_NAME_LEN];
	TimerAttrib attrib; 
} TIMER;

static TLock _timer_pv;
static ub _cur_hardware_alarm_ms = 0;
static ub _new_hardware_alarm_ms = 0;
static TIMERID _cur_creat_timer_id = 0;
static TIMER _timer[TIMER_MAX];
static sb _timer_id_reg[TIMER_MAX];
static ThreadId _timer_thread = INVALID_THREAD_ID;

static void
_timer_reset(TIMER *pTimer)
{
	pTimer->fun = NULL;
	pTimer->param_fun = NULL;
	pTimer->param = NULL;
	pTimer->owner = INVALID_THREAD_ID;
	
	pTimer->alarm_ms = 0;
	pTimer->life_ms = 0;
	pTimer->time_out_counter = 0;
	
	dave_memset(pTimer->name, 0x00, TIMER_NAME_LEN);
	pTimer->attrib = NONE_TIMER;
}

static void
_timer_run_function(base_timer_fun fun, base_timer_param_fun param_fun, void *param, TIMERID timer_id, ub wakeup_index)
{
	if(fun != NULL)
	{
		fun(timer_id, wakeup_index);
	}
	else if(param_fun != NULL)
	{
		param_fun(timer_id, wakeup_index, param);
	}
}

static RetCode
_timer_hardware_timer_notify(ub notify_id)
{
	TIMERMSG *pMsg;
	TIMERID reg_index, timer_id;

	for(reg_index=0; reg_index<TIMER_MAX; reg_index++)
	{
		if(_timer_id_reg[reg_index] == INVALID_TIMER_ID)
			break;
		timer_id = _timer_id_reg[reg_index];

		if((_timer[timer_id].fun != NULL) || (_timer[timer_id].param_fun != NULL))
		{
			if((_timer[timer_id].life_ms + _cur_hardware_alarm_ms) <= _timer[timer_id].life_ms)
				_timer[timer_id].life_ms = _timer[timer_id].alarm_ms;
			else
				_timer[timer_id].life_ms += _cur_hardware_alarm_ms;

			if(_timer[timer_id].life_ms >= _timer[timer_id].alarm_ms)
			{
				_timer[timer_id].life_ms = 0;

				if(_timer[timer_id].attrib == HW_TIMER)
				{
					_timer_run_function(_timer[timer_id].fun, _timer[timer_id].param_fun, _timer[timer_id].param, timer_id, 0);
				}
				else
				{
					pMsg = thread_msg(pMsg);

					pMsg->timer_id = (sb)timer_id;

					snd_from_msg(_timer_thread, _timer_thread, MSGID_TIMER, sizeof(TIMERMSG), pMsg);
				}
			}
		}
	}

	if(notify_id >= 1)
		return RetCode_Invalid_parameter;
	else
		return RetCode_OK;
}

static ub
_timer_get_divisor_alarm_ms(ub cur_alarm_ms, ub new_alarm_ms, s8 *name)
{
	ub tmp_alarm;

	if((cur_alarm_ms == 0) || (new_alarm_ms == 0))
	{
		TIMEABNOR("%s creat invalid timer: cur,%d new,%d", name, cur_alarm_ms, new_alarm_ms);
		return CREAT_SW_TIMER_MIN_VALUE;
	}

	if(cur_alarm_ms < new_alarm_ms)
	{
		tmp_alarm = cur_alarm_ms;
		cur_alarm_ms = new_alarm_ms;
		new_alarm_ms = tmp_alarm;
	}
	
	while(new_alarm_ms != 0)
	{
		tmp_alarm = cur_alarm_ms % new_alarm_ms;
		cur_alarm_ms = new_alarm_ms;
		new_alarm_ms = tmp_alarm;
	}

	if(cur_alarm_ms < CREAT_SW_TIMER_MIN_VALUE)
	{
		TIMEABNOR("%s creat invalid timer: cur,%d new,%d", name, cur_alarm_ms, new_alarm_ms);
		return CREAT_SW_TIMER_MIN_VALUE;
	}
	else
	{
		return cur_alarm_ms;
	}
}

static ub
_timer_get_alarm_ms(s8 *name)
{
	TIMERID reg_index, timer_id;
	ub alarm_ms;

	if(_timer_id_reg[0] == INVALID_TIMER_ID)
		return 0;

	timer_id = _timer_id_reg[0];
	alarm_ms =  _timer[timer_id].alarm_ms;

	for(reg_index=1; (reg_index<TIMER_MAX)&&(_timer_id_reg[reg_index]!=INVALID_TIMER_ID); reg_index++)
	{
		timer_id = _timer_id_reg[reg_index];
		TIMEDEBUG("time change:%d->%d", alarm_ms, _timer[timer_id].alarm_ms);
		alarm_ms = _timer_get_divisor_alarm_ms(alarm_ms, _timer[timer_id].alarm_ms, name);
	}

	return alarm_ms;
}

static void
_timer_opt_hardware_timer(TimerOPType op_type, s8 *name)
{
	ub divisor_alarm;

	TIMEDEBUG("type:%d name:%s time change:%d->%d",
		op_type, name,
		_cur_hardware_alarm_ms, _new_hardware_alarm_ms);

	if(op_type == CREATE_TIMER)
	{
		if(_cur_hardware_alarm_ms == 0)
		{
			_cur_hardware_alarm_ms = _new_hardware_alarm_ms;
			TIMEDEBUG("start hardware timer:%d", _cur_hardware_alarm_ms);
			if(dave_os_start_hardware_timer(_timer_hardware_timer_notify, _cur_hardware_alarm_ms) == dave_false)
			{
				base_restart("HARDWARE TIMER");
			}
		}
		else
		{
			TIMEDEBUG("time change:%d->%d", _cur_hardware_alarm_ms, _new_hardware_alarm_ms);

			divisor_alarm = _timer_get_divisor_alarm_ms(_cur_hardware_alarm_ms, _new_hardware_alarm_ms, name);
			if(divisor_alarm != 0)
			{
				if(_cur_hardware_alarm_ms != divisor_alarm)
				{
					TIMEDEBUG("stop hardware timer");
					dave_os_stop_hardware_timer();
					_cur_hardware_alarm_ms = divisor_alarm;
					TIMEDEBUG("start hardware timer:%d", _cur_hardware_alarm_ms);
					if(dave_os_start_hardware_timer(_timer_hardware_timer_notify, _cur_hardware_alarm_ms) == dave_false)
					{
						base_restart("HARDWARE TIMER");
					}
				}
			}
			else
			{
				_cur_hardware_alarm_ms = 0;
			}
		}
	}
	else if(op_type == DIE_TIMER)
	{
		divisor_alarm = _timer_get_alarm_ms(name);
		if(divisor_alarm != 0)
		{
			if(_cur_hardware_alarm_ms != divisor_alarm)
			{
				TIMEDEBUG("stop hardware timer");
				dave_os_stop_hardware_timer();
				_cur_hardware_alarm_ms = divisor_alarm;
				TIMEDEBUG("start hardware timer:%d", _cur_hardware_alarm_ms);
				if(dave_os_start_hardware_timer(_timer_hardware_timer_notify, _cur_hardware_alarm_ms) == dave_false)
				{
					base_restart("HARDWARE TIMER");
				}
			}
		}
		else
		{
			_cur_hardware_alarm_ms = 0;
		}
	}
	else if(op_type == STOP_TIMER)
	{
		TIMEDEBUG("stop hardware timer");
		dave_os_stop_hardware_timer();
		_cur_hardware_alarm_ms = 0;
		_new_hardware_alarm_ms = 0;
	}
	else
	{
		TIMEDEBUG("TimerOPType wrong: %d", op_type);
	}

	TIMEDEBUG("current alarm time = %d", _cur_hardware_alarm_ms);
}

static dave_bool
_timer_refresh_timer_id_reg(void)
{
	TIMERID reg_index, timer_id;
	dave_bool has_timer;

	for(reg_index=0; reg_index<TIMER_MAX; reg_index++)
	{
		_timer_id_reg[reg_index] = INVALID_TIMER_ID;
	}

	reg_index = 0;
	has_timer = dave_false;
	for(timer_id=0; timer_id<TIMER_MAX; timer_id++)
	{
		if((_timer[timer_id].fun != NULL) || (_timer[timer_id].param_fun != NULL))
		{
			_timer_id_reg[reg_index ++] = timer_id;
			has_timer = dave_true;
		}
	}

	return has_timer;
}

static void
_timer_event(MSGBODY *msg)
{
	TIMERID timer_id;
	TIMER *pTimer;
	base_timer_fun fun = NULL;
	base_timer_param_fun param_fun = NULL;
	void *param = NULL;

	timer_id = ((TIMERMSG *)(msg->msg_body))->timer_id;
	if(timer_id < TIMER_MAX)
	{
		pTimer = &_timer[timer_id];
	
		SAFECODEidlev1(pTimer->opt_pv, {
			fun = pTimer->fun;
			param_fun = pTimer->param_fun;
			param = pTimer->param;
		} );

		/*
		 * 此处确保用户层不会发生多线程竞争，
		 * 且通过判断_timer[timer_id].fun来
		 * 确保已经删除的定时器消息不会有机会执行。
		 */
		SAFECODEidlev1(pTimer->run_pv, {
			if((fun != NULL) || (param_fun != NULL))
			{
				pTimer->time_out_counter ++;

				_timer_run_function(fun, param_fun, param, timer_id, msg->thread_wakeup_index);
			}
		} );
	}
}

static dave_bool
_timer_the_thread_has_timer(ThreadId owner)
{
	ub index;

	if(owner != INVALID_THREAD_ID)
	{
		for(index=0; index<TIMER_MAX; index++)
		{
			if(owner == _timer[index].owner)
			{
				return dave_true;
			}
		}
	}

	return dave_false;
}

static TIMERID
_timer_creat_timer_(s8 *name, TimerAttrib attrib, ThreadId owner, base_timer_fun fun, void *param, ub alarm_ms)
{
	ub safe_count;
	TIMERID timer_id;
	dave_bool new_time_flag = dave_false;

	if((owner == INVALID_THREAD_ID) || (fun == NULL))
	{
		TIMEABNOR("owner=%d creat name:%s", owner, name);
		return INVALID_TIMER_ID;
	}

	for(safe_count=0; (new_time_flag==dave_false)&&(safe_count<TIMER_MAX); safe_count++)
	{	
		if((++ _cur_creat_timer_id) >= TIMER_MAX)
		{
			_cur_creat_timer_id = 0;
		}

		timer_id = _cur_creat_timer_id;

		SAFECODEv1(_timer[timer_id].opt_pv, {
			if((_timer[timer_id].fun == NULL)
				&& (_timer[timer_id].param_fun == NULL)
				&& (_timer[timer_id].owner == INVALID_THREAD_ID))
			{
				if(param == NULL)
					_timer[timer_id].fun = (base_timer_fun)fun;
				else
					_timer[timer_id].param_fun = (base_timer_param_fun)fun;
				_timer[timer_id].param = param;
				_timer[timer_id].owner = owner;

				_timer[timer_id].alarm_ms = alarm_ms;
				_timer[timer_id].life_ms = 0;
				_timer[timer_id].time_out_counter = 0;

				dave_strcpy(_timer[timer_id].name, name, TIMER_NAME_LEN);
				_timer[timer_id].attrib = attrib;

				new_time_flag = dave_true;
			}
		} );

		if(new_time_flag == dave_true)
		{
			if(attrib == SW_TIMER)
			{
				reg_msg(MSGID_TIMER, _timer_event);
			}

			return timer_id;
		}
	}

	return INVALID_TIMER_ID;
}

static void
_timer_die_timer_(TIMERID timer_id, ThreadId owner)
{
	dave_bool unreg_flag = dave_false;

	SAFECODEv1(_timer[timer_id].opt_pv, {
		_timer_reset(&_timer[timer_id]);

		if(_timer_the_thread_has_timer(owner) == dave_false)
		{
			unreg_flag = dave_true;
		}
	} );

	if(unreg_flag == dave_true)
	{
		unreg_msg(MSGID_TIMER);
	}
}

static TIMERID
_timer_creat_timer(s8 *name, TimerAttrib attrib, ThreadId owner, void *fun, void *param, ub alarm_ms)
{
	TIMERID timer_id;

	if(owner == INVALID_THREAD_ID)
	{
		TIMEABNOR("%s has invalid owner thread!", name);
		return INVALID_TIMER_ID;
	}

	/*
	 * 时间精度在秒级别。
	 */
	alarm_ms = (alarm_ms / 1000) * 1000;

	if(alarm_ms == 0)
	{
		TIMEABNOR("alarm_ms is zero!", alarm_ms);
		return INVALID_TIMER_ID;
	}

	TIMEDEBUG("alarm_ms:%d", alarm_ms);

	_new_hardware_alarm_ms = alarm_ms;

	timer_id = _timer_creat_timer_(name, attrib, owner, fun, param, alarm_ms);
	if(timer_id != INVALID_TIMER_ID)
	{
		_timer_refresh_timer_id_reg();

		_timer_opt_hardware_timer(CREATE_TIMER, name);
	}

	if((timer_id >= TIMER_MAX) || (timer_id == INVALID_TIMER_ID))
	{
		TIMEABNOR("timer:%s not creat!", name);
	}

	return timer_id;
}

static RetCode
_timer_die_timer(TIMERID timer_id)
{
	ThreadId owner = self();

	if(timer_id >= TIMER_MAX)
		return RetCode_Invalid_parameter;

	if(_timer[timer_id].owner != owner)
		return RetCode_Invalid_call;

	_timer_die_timer_(timer_id, owner);

	if(_timer_refresh_timer_id_reg() == dave_true)
		_timer_opt_hardware_timer(DIE_TIMER, _timer[timer_id].name);
	else
		_timer_opt_hardware_timer(STOP_TIMER, _timer[timer_id].name);

	return RetCode_OK;
}

static dave_bool
_timer_check_timer_is_recreat(s8 *name, base_timer_fun fun, base_timer_param_fun param_fun, TIMERID *time_id)
{
	TIMERID index;

	for(index=0; index<TIMER_MAX; index++)
	{
		if(((_timer[index].fun != NULL) || (_timer[index].param_fun != NULL))
			&& (dave_strcmp(_timer[index].name, name) == dave_true)
			&& (((_timer[index].fun == fun) && (fun != NULL)) || ((_timer[index].param_fun == param_fun) && (param_fun != NULL))))
		{
			if(time_id != NULL)
			{
				*time_id = index;
			}

			_timer[index].life_ms = 0;

			return dave_true;
		}
	}

	return dave_false;
}

static void
_timer_msg(TIMERMSG *pTimerMsg)
{
	TIMERMSG *pMsg;
	TIMERID timer_id;

	timer_id = pTimerMsg->timer_id;

	if(((_timer[timer_id].fun != NULL) || (_timer[timer_id].param_fun != NULL))
		&& (_timer[timer_id].attrib == SW_TIMER))
	{
		if(dave_strcmp(thread_name(_timer[timer_id].owner), "NULL") == dave_false)
		{
			pMsg = thread_msg(pMsg);

			pMsg->timer_id = timer_id;

			write_msg(_timer[timer_id].owner, MSGID_TIMER, pMsg);
		}
	}
}

static TIMERID
_timer_safe_creat_timer(s8 *name, TimerAttrib attrib, ThreadId owner, void *fun, void *param, ub alarm_ms)
{
	TIMERID timer_id = INVALID_TIMER_ID;

	SAFECODEv1(_timer_pv, { timer_id = _timer_creat_timer(name, attrib, owner, fun, param, alarm_ms); } );

	return timer_id;
}

static RetCode
_timer_safe_die_timer(TIMERID timer_id)
{
	RetCode ret = RetCode_Invalid_parameter;

	SAFECODEv1( _timer_pv, { ret = _timer_die_timer(timer_id); } );

	return ret;
}

static dave_bool
_timer_safe_check_timer_is_recreat(s8 *name, base_timer_fun fun, base_timer_param_fun param_fun, TIMERID *time_id)
{
	dave_bool ret = dave_false;

	SAFECODEv1( _timer_pv, { ret = _timer_check_timer_is_recreat(name, fun, param_fun, time_id); } );

	return ret;
}

static ub
_timer_info(s8 *info_ptr, ub info_len, s8 *owner)
{
	ub info_index, printf_len;
	TIMERID index;
	TIMER *pTimer;

	if(owner[0] == '\0')
	{
		owner = NULL;
	}

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		"TIMER INFORMATION:\n");

	for(index=0; index<TIMER_MAX; index++)
	{
		pTimer = &_timer[index];

		if((pTimer->fun != NULL) || (pTimer->param_fun != NULL))
		{
			if((owner == NULL)
				|| (dave_strcmp(owner, thread_name(pTimer->owner)) == dave_true))
			{
				printf_len = dave_snprintf(&info_ptr[info_index], info_len-info_index, " %s", pTimer->name);
				info_index += printf_len;

				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\t%s", printf_len < 8 ? "\t" : "");

				printf_len = dave_snprintf(&info_ptr[info_index], info_len-info_index, "%s", thread_name(pTimer->owner));
				info_index += printf_len;

				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\t%s", printf_len < 8 ? "\t" : "");

				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "alarm_ms:%ld life_ms:%ld time_out_counter:%lu\n",
					pTimer->alarm_ms, pTimer->life_ms, pTimer->time_out_counter);
			}
		}
	}

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		" -------------------\n HARDWARE TIMER:%ld",
		_cur_hardware_alarm_ms);

	return info_index;
}

static void
_timer_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_msg(pRsp);
	ub info_index = 0;

	switch(pReq->msg[0])
	{
		case 'i':
				info_index += _timer_info(pRsp->msg, sizeof(pRsp->msg), &(pReq->msg[1]));
			break;
		default:
				info_index += dave_snprintf(pRsp->msg, sizeof(pRsp->msg), "timer empty message!");
			break;
	}
	pRsp->ptr = pReq->ptr;

	write_msg(src, MSGID_DEBUG_RSP, pRsp);
}

static void
_timer_init(MSGBODY *task_msg)
{

}

static void
_timer_main(MSGBODY *msg)
{
	switch((ub)(msg->msg_id))
	{
		case MSGID_TIMER:
				_timer_msg((TIMERMSG *)(msg->msg_body));
			break;
		case MSGID_DEBUG_REQ:
				_timer_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_timer_exit(MSGBODY *task_msg)
{

}

// =====================================================================

TIMERID
base_timer_creat(char *name, base_timer_fun fun, ub alarm_ms)
{
	ThreadId owner;
	TIMERID timer_id;

	if(_timer_safe_check_timer_is_recreat((s8 *)name, fun, NULL, &timer_id) == dave_true)
	{
		return timer_id;
	}

	if(alarm_ms < CREAT_SW_TIMER_MIN_VALUE)
	{
		TIMEABNOR("%s creat min value(%d) timer name:%s", thread_name(get_self()), alarm_ms, name);
		alarm_ms = CREAT_SW_TIMER_MIN_VALUE;
	}

	if(alarm_ms > CREAT_SW_TIMER_MAX_VALUE)
	{
		TIMEABNOR("%s creat max value(%d) timer name:%s", thread_name(get_self()), alarm_ms, name);
		alarm_ms = CREAT_SW_TIMER_MAX_VALUE;
	}

	owner = get_self();

	timer_id = _timer_safe_creat_timer((s8 *)name, SW_TIMER, owner, fun, NULL, alarm_ms);

	TIMEDEBUG("name:%s %dms id:%d owner:%s<%d>",
		_timer[timer_id].name, _timer[timer_id].alarm_ms, timer_id,
		thread_name(owner), owner);

	return timer_id;
}

TIMERID
base_timer_param_creat(char *name, base_timer_param_fun fun, void *param, ub alarm_ms)
{
	ThreadId owner;
	TIMERID timer_id;

	if(_timer_safe_check_timer_is_recreat((s8 *)name, NULL, fun, &timer_id) == dave_true)
	{
		return timer_id;
	}

	if(alarm_ms < CREAT_SW_TIMER_MIN_VALUE)
	{
		TIMEABNOR("%s creat min value(%d) timer name:%s", thread_name(get_self()), alarm_ms, name);
		alarm_ms = CREAT_SW_TIMER_MIN_VALUE;
	}

	if(alarm_ms > CREAT_SW_TIMER_MAX_VALUE)
	{
		TIMEABNOR("%s creat max value(%d) timer name:%s", thread_name(get_self()), alarm_ms, name);
		alarm_ms = CREAT_SW_TIMER_MAX_VALUE;
	}

	owner = get_self();

	timer_id = _timer_safe_creat_timer((s8 *)name, SW_TIMER, owner, fun, param, alarm_ms);

	TIMEDEBUG("name:%s %dms id:%d owner:%s<%d>",
		_timer[timer_id].name, _timer[timer_id].alarm_ms, timer_id,
		thread_name(owner), owner);

	return timer_id;
}

RetCode
__base_timer_die__(TIMERID timer_id, s8 *fun, ub line)
{
	ThreadId cur_msg_id;

	if((timer_id >= TIMER_MAX) || (timer_id == INVALID_TIMER_ID))
	{
		TIMEABNOR("failed! timer_id:%d (%s:%d)", timer_id, fun, line);
		return RetCode_Invalid_parameter;
	}

	if(_timer[timer_id].attrib == SW_TIMER)
	{
		cur_msg_id = self();
		if(_timer[timer_id].owner != cur_msg_id)
		{
			TIMEABNOR("failed! (timer name:%s, owner task:%s<%d>, cur task:%s<%d>) (%s:%d)",
					_timer[timer_id].name,
					get_thread_name(_timer[timer_id].owner), _timer[timer_id].owner,
					get_thread_name(cur_msg_id), cur_msg_id,
					fun, line);
		}
	}

	TIMEDEBUG("name:%s %dms id:%d", _timer[timer_id].name, _timer[timer_id].alarm_ms, timer_id);

	return _timer_safe_die_timer(timer_id);
}

void
base_timer_init(void)
{
	TIMERID timer_id, reg_index;

	t_lock_reset(&_timer_pv);

	_cur_hardware_alarm_ms = 0;
	_new_hardware_alarm_ms = 0;
	_cur_creat_timer_id = 0;
	for(timer_id=0; timer_id<TIMER_MAX; timer_id++)
	{
		_timer[timer_id].timer_id = timer_id;

		t_lock_reset(&(_timer[timer_id].opt_pv));
		t_lock_reset(&(_timer[timer_id].run_pv));

		_timer_reset(&_timer[timer_id]);
	}
	for(reg_index=0; reg_index<TIMER_MAX; reg_index++)
	{
		_timer_id_reg[reg_index] = INVALID_TIMER_ID;
	}

	_timer_thread = base_thread_creat(TIMER_THREAD_NAME, 0, THREAD_MSG_WAKEUP|THREAD_PRIVATE_FLAG, _timer_init, _timer_main, _timer_exit);
	if(_timer_thread == INVALID_THREAD_ID)
		base_restart(TIMER_THREAD_NAME);
}

void
base_timer_exit(void)
{
	dave_os_stop_hardware_timer();

	if(_timer_thread != INVALID_THREAD_ID)
		base_thread_del(_timer_thread);
	_timer_thread = INVALID_THREAD_ID;
}

#endif

