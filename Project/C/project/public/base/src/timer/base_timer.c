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
#include "dave_verno.h"
#include "base_tools.h"
#include "timer_log.h"

#define BASE_TIMER_MAX (4096)
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
	ub timer_name_len;
	s8 *timer_name_ptr;

	TLock opt_pv;
	TLock run_pv;

	base_timer_fun fun;
	base_timer_param_fun param_fun;
	ub param_len;
	void *param_ptr;
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
	if(pTimer->timer_name_ptr != NULL)
	{
		dave_free(pTimer->timer_name_ptr);
		pTimer->timer_name_ptr = NULL;
	}

	pTimer->fun = NULL;
	pTimer->param_fun = NULL;
	if(pTimer->param_len >= sizeof(void *))
	{
		if(pTimer->param_len > sizeof(void *))
		{
			if(pTimer->param_ptr != NULL)
			{
				dave_free(pTimer->param_ptr);
			}
		}
		pTimer->param_ptr = NULL;
	}
	pTimer->owner = INVALID_THREAD_ID;
	
	pTimer->alarm_ms = 0;
	pTimer->life_ms = 0;
	pTimer->time_out_counter = 0;
}

static void
_timer_run_function(base_timer_fun fun, base_timer_param_fun param_fun, void *param_ptr, TIMERID timer_id, ub wakeup_index)
{
	if(fun != NULL)
	{
		fun(timer_id, wakeup_index);
	}
	else if(param_fun != NULL)
	{
		param_fun(timer_id, wakeup_index, param_ptr);
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
		if(_timer[timer_id].timer_name_ptr != NULL)
		{
			_timer_id_reg[reg_index ++] = _timer[timer_id].timer_id;
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
	void *param_ptr = NULL;

	timer_id = ((TIMERMSG *)(msg->msg_body))->timer_id;
	if(timer_id < BASE_TIMER_MAX)
	{
		current_time_ms = dave_os_time_ms();
		pTimer = &_timer[timer_id];
		fun = NULL;
		param_fun = NULL;

		SAFECODEidlev1(pTimer->opt_pv, {

			if((pTimer->timer_name_ptr != NULL)
				&& ((current_time_ms - pTimer->wakeup_time_ms) >= pTimer->alarm_ms))
			{
				pTimer->wakeup_time_ms = current_time_ms;

				fun = pTimer->fun;
				param_fun = pTimer->param_fun;
				param_ptr = pTimer->param_ptr;
			}

		} );

		if((fun != NULL) || (param_fun != NULL))
		{
			SAFECODEidlev1(pTimer->run_pv, {
					pTimer->time_out_counter ++;

					_timer_run_function(fun, param_fun, param_ptr, timer_id, msg->thread_wakeup_index);

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
	dave_os_file_write(CREAT_WRITE_FLAG, file_name, dave_os_file_len(READ_FLAG, file_name, -1), file_data_len, (u8 *)file_data_ptr);
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

	file_data_len = dave_snprintf(file_data_ptr, sizeof(file_data_ptr), "version:%s\n", dave_verno());
	_timer_assert_file(file_name, file_data_ptr, file_data_len);

	for(timer_id=0; timer_id<BASE_TIMER_MAX; timer_id++)
	{
		pTimer = &_timer[timer_id];
		if(pTimer->timer_name_ptr != NULL)
		{
			file_data_len = dave_snprintf(file_data_ptr, sizeof(file_data_ptr),
				"timer_id:%lu timer_name:%s owner:%s alarm_ms:%lu\n",
				pTimer->timer_id, pTimer->timer_name_ptr,
				thread_name(pTimer->owner), pTimer->alarm_ms);

			_timer_assert_file(file_name, file_data_ptr, file_data_len);
		}
	}

	file_data_len = dave_snprintf(file_data_ptr, sizeof(file_data_ptr), "timer resource exhausted!");
	_timer_assert_file(file_name, file_data_ptr, file_data_len);

	dave_os_power_off(file_data_ptr);
}

static void
_timer_msg(TIMERMSG *pTimerMsg)
{
	TIMERMSG *pMsg;
	TIMERID timer_id;

	timer_id = pTimerMsg->timer_id;
	if(timer_id >= BASE_TIMER_MAX)
	{
		TIMELOG("invalid timer_id:%d", timer_id);
		return;
	}

	if((_timer[timer_id].fun != NULL) || (_timer[timer_id].param_fun != NULL))
	{
		if(_timer[timer_id].owner == INVALID_THREAD_ID)
		{
			_timer[timer_id].owner = main_thread_id_get();

			TIMELOG("timer:%s/%d no owner, set to main thread:%s",
				_timer[timer_id].timer_name_ptr, timer_id,
				thread_name(_timer[timer_id].owner));
		}

		if(dave_strcmp(thread_name(_timer[timer_id].owner), "NULL") == dave_false)
		{
			pMsg = thread_msg(pMsg);

			pMsg->timer_id = timer_id;

			id_msg(_timer[timer_id].owner, MSGID_TIMER, pMsg);
		}
	}
}

static inline TIMERID
_timer_creat_timer_(s8 *name, ThreadId owner, base_timer_fun fun, void *param_ptr, ub param_len, ub alarm_ms)
{
	ub safe_counter;
	TIMER *pTimer;
	dave_bool new_time_flag = dave_false;
	ub name_len;

	if(fun == NULL)
	{
		TIMEABNOR("owner:%s creat name:%s give invalid fun!", thread_name(owner), name);
		return INVALID_TIMER_ID;
	}

	for(safe_counter=0; (new_time_flag==dave_false)&&(safe_counter<BASE_TIMER_MAX); safe_counter++)
	{	
		if((++ _cur_creat_timer_id) >= BASE_TIMER_MAX)
		{
			_cur_creat_timer_id = 0;
		}
		pTimer = &_timer[_cur_creat_timer_id];

		SAFECODEv1(pTimer->opt_pv, {
			if((pTimer->fun == NULL)
				&& (pTimer->param_fun == NULL))
			{
				if(param_ptr == NULL)
				{
					pTimer->fun = (base_timer_fun)fun;
					pTimer->param_fun = NULL;
					pTimer->param_len = 0;
					pTimer->param_ptr = NULL;
				}
				else
				{
					pTimer->param_fun = (base_timer_param_fun)fun;
					if(param_len > sizeof(void *))
					{
						pTimer->param_len = param_len;
						pTimer->param_ptr = dave_malloc(pTimer->param_len);

						dave_memcpy(pTimer->param_ptr, param_ptr, pTimer->param_len);
					}
					else
					{
						pTimer->param_len = sizeof(void *);
						pTimer->param_ptr = param_ptr;
					}
				}
				pTimer->owner = owner;

				pTimer->wakeup_time_ms = dave_os_time_ms();

				pTimer->alarm_ms = alarm_ms;
				pTimer->life_ms = 0;
				pTimer->time_out_counter = 0;

				name_len = dave_strlen(name) + 1;
				pTimer->timer_name_ptr = dave_malloc(name_len);
				pTimer->timer_name_len = dave_strcpy(pTimer->timer_name_ptr, name, name_len);

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
_timer_creat_timer(s8 *name, ThreadId owner, void *fun, void *param_ptr, ub param_len, ub alarm_ms)
{
	TIMERID timer_id;

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

	timer_id = _timer_creat_timer_(name, owner, fun, param_ptr, param_len, alarm_ms);
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

static inline TIMERID
_timer_recreat_timer(TIMERID timer_id, void *fun, void *param_ptr, ub param_len, ub alarm_ms)
{
	TIMER *pTimer;

	if(timer_id >= BASE_TIMER_MAX)
	{
		return INVALID_TIMER_ID;
	}

	pTimer = &_timer[timer_id];

	SAFECODEv1(pTimer->opt_pv, {

		if(pTimer->param_ptr != NULL)
		{
			if(pTimer->param_len > sizeof(void *))
			{
				dave_free(pTimer->param_ptr);
			}
		}

		if((param_ptr != NULL) && (param_len > 0))
		{
			pTimer->param_fun = (base_timer_param_fun)fun;
			if(param_len > sizeof(void *))
			{
				pTimer->param_len = param_len;
				pTimer->param_ptr = dave_malloc(pTimer->param_len);
				dave_memcpy(pTimer->param_ptr, param_ptr, pTimer->param_len);
			}
			else
			{
				pTimer->param_len = sizeof(void *);
				pTimer->param_ptr = param_ptr;
			}
		}
		else
		{
			pTimer->param_fun = NULL;
			pTimer->fun = fun;
			pTimer->param_len = 0;
			pTimer->param_ptr = NULL;
		}

		pTimer->alarm_ms = alarm_ms;
	} );

	return timer_id;
}

static inline RetCode
_timer_die_timer(ub *param_len, void **param_ptr, TIMERID timer_id)
{
	ThreadId owner = self();

	if(timer_id >= BASE_TIMER_MAX)
		return RetCode_Invalid_parameter;

	if(_timer[timer_id].owner != owner)
		return RetCode_Invalid_call;

	if(param_len != NULL)
	{
		*param_len = _timer[timer_id].param_len;
	}
	if(param_ptr != NULL)
	{
		*param_ptr = _timer[timer_id].param_ptr;
	}

	_timer_die_timer_(timer_id, owner);

	if(_timer_refresh_timer_id_reg() == dave_true)
		_timer_opt_hardware_timer(DIE_TIMER, _timer[timer_id].timer_name_ptr);
	else
		_timer_opt_hardware_timer(STOP_TIMER, _timer[timer_id].timer_name_ptr);

	return RetCode_OK;
}

static inline TIMERID
_timer_name_to_id(s8 *name)
{
	ub name_len = dave_strlen(name);
	ub reg_index;
	TIMERID timer_id;
	TIMER *pTimer;

	for(reg_index=0; reg_index<BASE_TIMER_MAX; reg_index++)
	{
		timer_id = _timer_id_reg[reg_index];
		if((timer_id <= -1) || (timer_id >= BASE_TIMER_MAX))
		{
			break;
		}
		pTimer = &_timer[timer_id];

		if((pTimer->timer_name_ptr != NULL)
			&& (pTimer->timer_name_len == name_len)
			&& (dave_strcmp(pTimer->timer_name_ptr, name) == dave_true))
		{
			return pTimer->timer_id;
		}
	}

	return INVALID_TIMER_ID;
}

static inline TIMERID
_timer_check_recreat(s8 *name)
{
	return _timer_name_to_id(name);
}

static TIMERID
_timer_safe_creat_timer(s8 *name, ThreadId owner, void *fun, void *param_ptr, ub param_len, ub alarm_ms)
{
	TIMERID timer_id = INVALID_TIMER_ID;

	if((name == NULL) || (dave_strlen(name) == 0))
	{
		TIMEABNOR("invalid name:%s", name);
		return INVALID_TIMER_ID;
	}

	SAFECODEv1(_timer_pv, {

		timer_id = _timer_check_recreat(name);
		if(timer_id == INVALID_TIMER_ID)
		{
			timer_id = _timer_creat_timer(name, owner, fun, param_ptr, param_len, alarm_ms);
		}
		else
		{
			timer_id = _timer_recreat_timer(timer_id, fun, param_ptr, param_len, alarm_ms);
		}

	} );

	if(timer_id == INVALID_TIMER_ID)
	{
		_timer_assert();
	}

	return timer_id;
}

static RetCode
_timer_safe_die_timer(ub *param_len, void **param_ptr, TIMERID timer_id)
{
	RetCode ret = RetCode_Invalid_parameter;

	SAFECODEv1( _timer_pv, { ret = _timer_die_timer(param_len, param_ptr, timer_id); } );

	return ret;
}

static ub
_timer_info(s8 *info_ptr, ub info_len, s8 *owner)
{
	ub info_index, printf_len;
	TIMERID index;
	TIMER *pTimer;
	ub total_time_number;

	if(owner[0] == '\0')
	{
		owner = NULL;
	}

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		"TIMER INFORMATION:\n");

	total_time_number = 0;

	for(index=0; index<BASE_TIMER_MAX; index++)
	{
		pTimer = &_timer[index];

		if((pTimer->fun != NULL) || (pTimer->param_fun != NULL))
		{
			if((owner == NULL)
				|| (dave_strcmp(owner, thread_name(pTimer->owner)) == dave_true))
			{
				printf_len = dave_snprintf(&info_ptr[info_index], info_len-info_index, " %s", pTimer->timer_name_ptr);
				info_index += printf_len;

				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\t%s",
					printf_len < 8 ? "\t\t\t" : printf_len < 16 ? "\t\t" : printf_len < 24 ? "\t" : "");

				printf_len = dave_snprintf(&info_ptr[info_index], info_len-info_index, "%s", thread_name(pTimer->owner));
				info_index += printf_len;

				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\t%s",
					printf_len < 8 ? "\t\t" : printf_len < 16 ? "\t" : "");

				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "timer_id:%lu alarm_ms:%lu life_ms:%lu time_out_counter:%lu\n",
					pTimer->timer_id, pTimer->alarm_ms, pTimer->life_ms, pTimer->time_out_counter);

				total_time_number ++;
			}
		}
	}

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		" -------------------\n HARDWARE TIMER:%ld TIMER CAPACITY:%d/%d",
		_cur_hardware_alarm_ms, total_time_number, BASE_TIMER_MAX);

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

static ThreadId
_timer_owner(void)
{
	ThreadId owner = get_self();

	if(owner == INVALID_THREAD_ID)
	{
		owner = main_thread_id_get();
	}

	return owner;
}

static inline void
_timer_pre_(void)
{
	TIMERID timer_id, reg_index;

	t_lock_reset(&_timer_pv);
	_cur_hardware_alarm_ms = 0;
	_new_hardware_alarm_ms = 0;
	_cur_creat_timer_id = 0;
	for(timer_id=0; timer_id<BASE_TIMER_MAX; timer_id++)
	{
		_timer[timer_id].timer_id = timer_id;
		_timer[timer_id].timer_name_len = 0;
		_timer[timer_id].timer_name_ptr = NULL;
		_timer[timer_id].param_len = 0;
		_timer[timer_id].param_ptr = NULL;
	
		t_lock_reset(&(_timer[timer_id].opt_pv));
		t_lock_reset(&(_timer[timer_id].run_pv));
	
		_timer_reset(&_timer[timer_id]);
	}
	for(reg_index=0; reg_index<BASE_TIMER_MAX; reg_index++)
	{
		_timer_id_reg[reg_index] = INVALID_TIMER_ID;
	}
}

static inline void
_timer_pre(void)
{
	static volatile sb __safe_pre_flag__ = 0;

	SAFEPre(__safe_pre_flag__, { _timer_pre_(); });
}

static inline void
_timer_end(void)
{
	ub timer_id;

	dave_os_stop_hardware_timer();

	for(timer_id=0; timer_id<BASE_TIMER_MAX; timer_id++)
	{
		_timer_reset(&_timer[timer_id]);
	}
}

static RetCode
_timer_die(ub *param_len, void **param_ptr, TIMERID timer_id, s8 *fun, ub line)
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
		TIMEABNOR("%s die timer:%d/%s, owner:%s<%lx>, cur:%s<%lx> (%s:%d)",
				thread_name(cur_msg_id),
				timer_id, _timer[timer_id].timer_name_ptr,
				thread_name(_timer[timer_id].owner), _timer[timer_id].owner,
				thread_name(cur_msg_id), cur_msg_id,
				fun, line);
	}

	TIMEDEBUG("timer_name:%s %dms id:%d",
		_timer[timer_id].timer_name_ptr,
		_timer[timer_id].alarm_ms,
		timer_id);

	return _timer_safe_die_timer(param_len, param_ptr, timer_id);
}

// =====================================================================

TIMERID
base_timer_creat(char *name, base_timer_fun fun, ub alarm_ms)
{
	_timer_pre();

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

	return _timer_safe_creat_timer(name, _timer_owner(), fun, NULL, 0, alarm_ms);
}

TIMERID
base_timer_param_creat(char *name, base_timer_param_fun fun, void *param_ptr, ub param_len, ub alarm_ms)
{
	_timer_pre();

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

	if(param_ptr == NULL)
	{
		param_ptr = fun;
	}
	if(param_len == 0)
	{
		param_len = sizeof(fun);
	}

	return _timer_safe_creat_timer(name, _timer_owner(), fun, param_ptr, param_len, alarm_ms);
}

RetCode
__base_timer_die__(TIMERID timer_id, s8 *fun, ub line)
{
	_timer_pre();

	return _timer_die(NULL, NULL, timer_id, fun, line);
}

void *
__base_timer_kill__(char *name, s8 *fun, ub line)
{
	TIMERID timer_id;
	void *param_ptr = NULL;

	_timer_pre();

	timer_id = _timer_name_to_id(name);

	if(timer_id != INVALID_TIMER_ID)
	{
		_timer_die(NULL, &param_ptr, timer_id, fun, line);
	}

	return param_ptr;
}

void
base_timer_init(void)
{
	_timer_pre();

	_timer_thread = base_thread_creat(TIMER_THREAD_NAME, 0, THREAD_MSG_WAKEUP|THREAD_PRIVATE_FLAG|THREAD_CORE_FLAG, _timer_init, _timer_main, _timer_exit);
	if(_timer_thread == INVALID_THREAD_ID)
		base_restart(TIMER_THREAD_NAME);
}

void
base_timer_exit(void)
{
	if(_timer_thread != INVALID_THREAD_ID)
		base_thread_del(_timer_thread);
	_timer_thread = INVALID_THREAD_ID;

	_timer_end();
}

#endif

