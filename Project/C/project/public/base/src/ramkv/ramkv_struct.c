/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_os.h"
#include "dave_tools.h"
#include "ramkv_param.h"
#include "ramkv_struct.h"
#include "ramkv_timer.h"
#include "ramkv_local_struct.h"
#include "ramkv_remote_struct.h"
#include "ramkv_log.h"

static ub _ramkv_struct_global_init = 0x0123456789abcdef;
static TLock _ramkv_struct_global_pv;

static void
___ramkv_pv_booting___(void)
{
	t_lock_spin(NULL);
	if(_ramkv_struct_global_init == 0x0123456789abcdef)
	{
		_ramkv_struct_global_init = 0;

		t_lock_reset(&_ramkv_struct_global_pv);
	}
	t_unlock_spin(NULL);
}

static KV *
___ramkv_malloc___(s8 *name, KvAttrib attrib, ub out_second, ramkv_time_callback callback_fun, s8 *fun, ub line)
{
	KV *pKV;

	pKV = (KV *)ramkvm_malloc_line(sizeof(KV), fun, line);

	dave_memset(pKV, 0x00, sizeof(KV));

	pKV->magic_data = pKV->magic_rand = t_rand();

	t_lock_reset(&(pKV->ramkv_pv));

	dave_strcpy(pKV->thread_name, thread_name(self()), DAVE_THREAD_NAME_LEN);

	dave_strcpy(pKV->name, name, RAMKV_NAME_MAX);
	pKV->attrib = attrib;

	switch(pKV->attrib)
	{
		case KvAttrib_ram:
		case KvAttrib_list:
				ramkv_malloc_local(pKV);
			break;
		case KvAttrib_remote:
				ramkv_malloc_remote(pKV, pKV->name, pKV->attrib);
			break;
		default:
			break;
	}

	ramkv_timer_init(pKV, out_second, callback_fun);

	return pKV;
}

static void
___ramkv_free___(KV *pKV, s8 *fun, ub line)
{
	if(pKV == NULL)
		return;

	if(ramkv_check(pKV) == dave_false)
		return;

	if(dave_strcmp(pKV->thread_name, thread_name(self())) == dave_false)
	{
		KVABNOR("The resource requested by %s is released by %s! <%s:%d>",
			pKV->thread_name, thread_name(self()),
			fun, line);
	}

	ramkv_timer_exit(pKV);

	ramkv_free_local(pKV);

	ramkv_free_remote(pKV, pKV->attrib);

	dave_memset(pKV, 0x00, sizeof(KV));

	ramkvm_free(pKV);
}

// ====================================================================

void
ramkv_struct_init(void)
{
	___ramkv_pv_booting___();
}

void
ramkv_struct_exit(void)
{

}

KV *
__ramkv_malloc__(dave_bool external_call, s8 *name, KvAttrib attrib, ub out_second, ramkv_time_callback callback_fun, s8 *fun, ub line)
{
	KV *pKV = NULL;

	___ramkv_pv_booting___();

	if(external_call == dave_true)
	{
		SAFECODEv2W(_ramkv_struct_global_pv, pKV = ___ramkv_malloc___(name, attrib, out_second, callback_fun, fun, line););
	}
	else
	{
		pKV = ___ramkv_malloc___(name, attrib, out_second, callback_fun, fun, line);
	}

	return pKV;
}

void
__ramkv_free__(dave_bool external_call, KV *pKV, s8 *fun, ub line)
{
	___ramkv_pv_booting___();

	if(external_call == dave_true)
	{
		SAFECODEv2W(_ramkv_struct_global_pv, ___ramkv_free___(pKV, fun, line););
	}
	else
	{
		___ramkv_free___(pKV, fun, line);
	}
}

void
ramkv_timer(TIMERID timer_id, ub thread_index, void *param)
{
	KV *pKV = param;
	dave_bool kv_check_flag = dave_true;

	/*
	 * 时间事件会在线程空间排队，如果排队的过程中KV已经被释放了，
	 * 那么这个时候param携带的pKV指针就无效了，它会被其他模块使用。
	 * 这个时候如果使用param是一个危险行为。
	 */
	SAFECODEv2W(_ramkv_struct_global_pv, {

		kv_check_flag = ramkv_check(pKV);
		if(kv_check_flag == dave_true)
		{
			ramkv_timer_out(pKV);
		}

	} );

	if(kv_check_flag == dave_false)
	{
		KVABNOR("kv check failed!");
	}
}

dave_bool
__ramkv_check__(KV *pKV, s8 *fun, ub line)
{
	if(pKV == NULL)
	{
		KVLOG("empty pKV! <%s:%d>", fun, line);
		return dave_false;
	}

	if(pKV->magic_data == pKV->magic_rand)
	{
		return dave_true;
	}
	else
	{
		KVLOG("magic mismatch! %d!=%d <%s:%d>",
			pKV->magic_data, pKV->magic_rand,
			fun, line);
		return dave_false;
	}
}

#endif

