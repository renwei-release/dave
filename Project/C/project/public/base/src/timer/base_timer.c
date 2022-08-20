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

#define BASE_TIMER_MAX (8192)
#define TIMER_NAME_LEN (16)
#define CREAT_SW_TIMER_MIN_VALUE (1000)						// ms
#define CREAT_SW_TIMER_MAX_VALUE (1000*60*60*24*7UL)		// ms

typedef enum {
	CREATE_TIMER,
	DIE_TIMER,
	STOP_TIMER,

	OP_TIMER_NULL
} TimerOPType;

typedef struct {
	TIMERID timer_id;
	s8 name[TIMER_NAME_LEN];

	TLock opt_pv;
	TLock run_pv;

	base_timer_fun fun;
	base_timer_param_fun param_fun;
	void *param;
	ThreadId owner;

	ub wakeup_time_ms;

	ub alarm_ms;
	ub life_ms;
	ub time_out_counter;
} TIMER;

static TLock _timer_pv;
static ub _cur_hardware_alarm_ms = 0;
static ub _new_hardware_alarm_ms = 0;
static TIMERID _cur_creat_timer_id = 0;
static TIMER _timer[BASE_TIMER_MAX];
static sb _timer_id_reg[BASE_TIMER_MAX];
static ThreadId _timer_thread = INVALID_THREAD_ID;

static void
_timer_reset(TIMER *pTimer)
{
	dave_memset(pTimer->name, 0x00, TIMER_NAME_LEN);

	pTimer->fun = NULL;
	pTimer->param_fun = NULL;
	pTimer->param = NULL;
	pTimer->owner = INVALID_THREAD_ID;
	
	pTimer->alarm_ms = 0;
	pTimer->life_ms = 0;
	pTimer->time_out_counter = 0;
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
	TIMER *pTimer;
	TIMERID reg_index, timer_id;

	for(reg_index=0; reg_index<BASE_TIMER_MAX; reg_index++)
	{
		if(_timer_id_reg[reg_index] == INVALID_TIMER_ID)
			break;
		timer_id = _timer_id_reg[reg_index];

		pTimer = &_timer[timer_id];

		if((pTimer->fun != NULL) || (pTimer->param_fun != NULL))
		{
			if((pTimer->life_ms + _cur_hardware_alarm_ms) <= pTimer->life_ms)
				pTimer->life_ms = pTimer->alarm_ms;
			else
				pTimer->life_ms += _cur_hardware_alarm_ms;

			if(pTimer->life_ms >= pTimer->alarm_ms)
			{
				pTimer->life_ms = 0;

				pMsg = thread_msg(pMsg);
				pMsg->timer_id = (sb)timer_id;
				snd_from_msg(_timer_thread, _timer_thread, MSGID_TIMER, sizeof(TIMERMSG), pMsg);
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

	for(reg_index=1; (reg_index<BASE_TIMER_MAX)&&(_timer_id_reg[reg_index]!=INVALID_TIMER_ID); reg_index++)
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
				dave_os_stop_hardware_timer();
				_cur_hardware_alarm_ms = divisor_alarm;
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
		dave_os_stop_hardware_timer();
		_cur_hardware_alarm_ms = 0;
		_new_hardware_alarm_ms = 0;
	}

	TIMEDEBUG("current alarm time = %d", _cur_hardware_alarm_ms);
}

static dave_bool
_timer_refresh_timer_id_reg(void)
{
	TIMERID reg_index, timer_id;
	dave_bool has_timer;

	for(reg_index=0; reg_index<BASE_TIMER_MAX; reg_index++)
	{
		_timer_id_reg[reg_index] = INVALID_TIMER_ID;
	}

	reg_index = 0;
	has_timer = dave_false;
	for(timer_id=0; timer_id<BASE_TIMER_MAX; timer_id++)
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
	ub current_time_ms;
	TIMERID timer_id;
	TIMER *pTimer;
	base_timer_fun fun;
	base_timer_param_fun param_fun;
	void *param = NULL;

	timer_id = ((TIMERMSG *)(msg->msg_body))->timer_id;
	if(timer_id < BASE_TIMER_MAX)
	{
		current_time_ms = dave_os_time_ms();
		pTimer = &_timer[timer_id];
		fun = NULL;
		param_fun = NULL;

		SAFECODEidlev1(pTimer->opt_pv, {

			if((pTimer->name[0] != '\0')
				&& ((current_time_ms - pTimer->wakeup_time_ms) >= pTimer->alarm_ms))
			{
				pTimer->wakeup_time_ms = current_time_ms;

				fun = pTimer->fun;
				param_fun = pTimer->param_fun;
				param = pTimer->param;
			}

		} );

		/*
		 * 此处确保用户层不会发生多线程竞争，
		 * 且通过判断_timer[timer_id].fun来
		 * 确保已经删除的定时器消息不会有机会执行。
		 */

		if((fun != NULL) || (param_fun != NULL))
		{
			SAFECODEidlev1(pTimer->run_pv, {

					pTimer->time_out_counter ++;

					_timer_run_function(fun, param_fun, param, timer_id, msg->thread_wakeup_index);

			} );
		}
	}
}

static dave_bool
_timer_the_thread_has_timer(ThreadId owner)
{
	ub index;

	if(owner != INVALID_THREAD_ID)
	{
		for(index=0; index<BASE_TIMER_MAX; index++)
		{
			if(owner == _timer[index].owner)
			{
				return dave_true;
			}
		}
	}

	return dave_false;
}

static void
_timer_assert_file(s8 *file_name, s8 *file_data_ptr, ub file_data_len)
{
	dave_os_file_write(CREAT_WRITE_FLAG, file_name, dave_os_file_len(file_name, -1), file_data_len, (u8 *)file_data_ptr);
}

static void
_timer_assert(void)
{
	DateStruct date;
	s8 file_name[64];
	ub timer_id;
	TIMER *pTimer;
	s8 file_data_ptr[1024];
	ub file_data_len;

	date = t_time_get_date(NULL);

	dave_snprintf(file_name, sizeof(file_name), "TIMER-DUMP-%04d-%02d-%02d_%02d:%02d:%02d",
		date.year, date.month, date.day,
		date.hour, date.minute, date.second);

	for(timer_id=0; timer_id<BASE_TIMER_MAX; timer_id++)
	{
		pTimer = &_timer[timer_id];
		if(pTimer->name[0] != '\0')
		{
			file_data_len = dave_snprintf(file_data_ptr, sizeof(file_data_ptr),
				"timer_id:%lu name:%s owner:%s alarm_ms:%lu\n",
				pTimer->timer_id, pTimer->name,
				thread_name(pTimer->owner), pTimer->alarm_ms);

			_timer_assert_file(file_name, file_data_ptr, file_data_len);
		}
	}

	dave_os_power_off("timer resource exhausted!");
}

static void
_timer_msg(TIMERMSG *pTimerMsg)
{
	TIMERMSG *pMsg;
	TIMERID timer_id;

	timer_id = pTimerMsg->timer_id;

	if((_timer[timer_id].fun != NULL) || (_timer[timer_id].param_fun != NULL))
	{
		if(dave_strcmp(thread_name(_timer[timer_id].owner), "NULL") == dave_false)
		{
			pMsg = thread_msg(pMsg);

			pMsg->timer_id = timer_id;

			id_msg(_timer[timer_id].owner, MSGID_TIMER, pMsg);
		}
	}
}

static inline TIMERID
_timer_creat_timer_(s8 *name, ThreadId owner, base_timer_fun fun, void *param, ub alarm_ms)
{
	ub safe_count;
	TIMER *pTimer;
	dave_bool new_time_flag = dave_false;

	if((owner == INVALID_THREAD_ID) || (fun == NULL))
	{
		TIMEABNOR("owner=%d creat name:%s", owner, name);
		return INVALID_TIMER_ID;
	}

	for(safe_count=0; (new_time_flag==dave_false)&&(safe_count<BASE_TIMER_MAX); safe_count++)
	{	
		if((++ _cur_creat_timer_id) >= BASE_TIMER_MAX)
		{
			_cur_creat_timer_id = 0;
		}
		pTimer = &_timer[_cur_creat_timer_id];

		SAFECODEv1(pTimer->opt_pv, {
			if((pTimer->fun == NULL)
				&& (pTimer->param_fun == NULL)
				&& (pTimer->owner == INVALID_THREAD_ID))
			{
				if(param == NULL)
					pTimer->fun = (base_timer_fun)fun;
				else
					pTimer->param_fun = (base_timer_param_fun)fun;
				pTimer->param = param;
				pTimer->owner = owner;

				pTimer->wakeup_time_ms = dave_os_time_ms();

				pTimer->alarm_ms = alarm_ms;
				pTimer->life_ms = 0;
				pTimer->time_out_counter = 0;

				dave_strcpy(pTimer->name, name, TIMER_NAME_LEN);

				new_time_flag = dave_true;
			}
		} );

		if(new_time_flag == dave_true)
		{
			reg_msg(MSGID_TIMER, _timer_event);

			return _cur_creat_timer_id;
		}
	}

	return INVALID_TIMER_ID;
}

static inline void
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

static inline TIMERID
_timer_creat_timer(s8 *name, ThreadId owner, void *fun, void *param, ub alarm_ms)
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

	timer_id = _timer_creat_timer_(name, owner, fun, param, alarm_ms);
	if(timer_id != INVALID_TIMER_ID)
	{
		_timer_refresh_timer_id_reg();

		_timer_opt_hardware_timer(CREATE_TIMER, name);
	}

	if((timer_id >= BASE_TIMER_MAX) || (timer_id == INVALID_TIMER_ID))
	{
		TIMEABNOR("timer:%s not creat!", name);
	}

	return timer_id;
}

static inline RetCode
_timer_die_timer(TIMERID timer_id)
{
	ThreadId owner = self();

	if(timer_id >= BASE_TIMER_MAX)
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

static inline TIMERID
_timer_check_recreat(s8 *name)
{
	ub reg_index;
	TIMERID timer_id;
	TIMER *pTimer;

	for(reg_index=0; reg_index<BASE_TIMER_MAX; reg_index++)
	{
		timer_id = _timer_id_reg[reg_index];
		if((timer_id <= -1) || (timer_id >= BASE_TIMER_MAX))
			break;
		pTimer = &_timer[timer_id];

		if(((pTimer->fun != NULL) || (pTimer->param_fun != NULL))
			&& (dave_strcmp(pTimer->name, name) == dave_true))
		{
			return pTimer->timer_id;
		}
	}

	return INVALID_TIMER_ID;
}

static TIMERID
_timer_safe_creat_timer(s8 *name, ThreadId owner, void *fun, void *param, ub alarm_ms)
{
	TIMERID timer_id = INVALID_TIMER_ID;

	SAFECODEv1(_timer_pv, {

		timer_id = _timer_check_recreat(name);
		if(timer_id == INVALID_TIMER_ID)
		{
			timer_id = _timer_creat_timer(name, owner, fun, param, alarm_ms);
		}

	} );

	if(timer_id == INVALID_TIMER_ID)
	{
		_timer_assert();
	}

	return timer_id;
}

static RetCode
_timer_safe_die_timer(TIMERID timer_id)
{
	RetCode ret = RetCode_Invalid_parameter;

	SAFECODEv1( _timer_pv, { ret = _timer_die_timer(timer_id); } );

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

	for(index=0; index<BASE_TIMER_MAX; index++)
	{
		pTimer = &_timer[index];

		if((pTimer->fun != NULL) || (pTimer->param_fun != NULL))
		{
			if((owner == NULL)
				|| (dave_strcmp(owner, thread_name(pTimer->owner)) == dave_true))
			{
				printf_len = dave_snprintf(&info_ptr[info_index], info_len-info_index, " %s", pTimer->name);
				info_index += printf_len;

				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\t%s",
					printf_len < 8 ? "\t\t" : printf_len < 16 ? "\t" : "");

				printf_len = dave_snprintf(&info_ptr[info_index], info_len-info_index, "%s", thread_name(pTimer->owner));
				info_index += printf_len;

				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\t%s",
					printf_len < 8 ? "\t\t" : printf_len < 16 ? "\t" : "");

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

	id_msg(src, MSGID_DEBUG_RSP, pRsp);
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

	timer_id = _timer_safe_creat_timer((s8 *)name, owner, fun, NULL, alarm_ms);

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

	timer_id = _timer_safe_creat_timer((s8 *)name, owner, fun, param, alarm_ms);

	TIMEDEBUG("name:%s %dms id:%d owner:%s<%d>",
		_timer[timer_id].name, _timer[timer_id].alarm_ms, timer_id,
		thread_name(owner), owner);

	return timer_id;
}

RetCode
__base_timer_die__(TIMERID timer_id, s8 *fun, ub line)
{
	ThreadId cur_msg_id;

	if((timer_id >= BASE_TIMER_MAX) || (timer_id == INVALID_TIMER_ID))
	{
		TIMEABNOR("failed! timer_id:%d (%s:%d)", timer_id, fun, line);
		return RetCode_Invalid_parameter;
	}

	cur_msg_id = self();
	if(_timer[timer_id].owner != cur_msg_id)
	{
		TIMEABNOR("failed! (timer name:%s, owner task:%s<%d>, cur task:%s<%d>) (%s:%d)",
				_timer[timer_id].name,
				get_thread_name(_timer[timer_id].owner), _timer[timer_id].owner,
				get_thread_name(cur_msg_id), cur_msg_id,
				fun, line);
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
	for(timer_id=0; timer_id<BASE_TIMER_MAX; timer_id++)
	{
		_timer[timer_id].timer_id = timer_id;

		t_lock_reset(&(_timer[timer_id].opt_pv));
		t_lock_reset(&(_timer[timer_id].run_pv));

		_timer_reset(&_timer[timer_id]);
	}
	for(reg_index=0; reg_index<BASE_TIMER_MAX; reg_index++)
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

