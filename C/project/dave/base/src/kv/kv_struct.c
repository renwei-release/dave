/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_os.h"
#include "dave_tools.h"
#include "kv_param.h"
#include "kv_struct.h"
#include "kv_timer.h"
#include "kv_local_struct.h"
#include "kv_remote_struct.h"
#include "kv_log.h"

static ub _kv_struct_global_init = 0x0123456789abcdef;
static DaveLock _kv_struct_global_pv;

static void
___kv_pv_booting___(void)
{
	t_lock_spin(NULL);
	if(_kv_struct_global_init == 0x0123456789abcdef)
	{
		_kv_struct_global_init = 0;

		dave_lock_reset(&_kv_struct_global_pv);
	}
	t_unlock_spin(NULL);
}

static KV *
___kv_malloc___(s8 *name, KVAttrib attrib, ub out_second, kv_out_callback callback_fun, s8 *fun, ub line)
{
	KV *pKV;

	pKV = kvm_malloc_line(sizeof(KV), fun, line);

	dave_memset(pKV, 0x00, sizeof(KV));

	pKV->magic_data = pKV->magic_rand = dave_rand();

	dave_lock_reset(&(pKV->kv_pv));

	dave_strcpy(pKV->thread_name, thread_name(self()), DAVE_THREAD_NAME_LEN);

	dave_strcpy(pKV->name, name, KV_NAME_MAX);
	pKV->attrib = attrib;

	switch(pKV->attrib)
	{
		case KVAttrib_ram:
		case KVAttrib_list:
				kv_malloc_local(pKV);
			break;
		case KVAttrib_remote:
				kv_malloc_remote(pKV, pKV->name, pKV->attrib);
			break;
		default:
			break;
	}

	kv_timer_init(pKV, out_second, callback_fun);

	return pKV;
}

void
___kv_free___(KV *pKV)
{
	if(pKV == NULL)
		return;

	if(kv_check(pKV) == dave_false)
		return;

	if(dave_strcmp(pKV->thread_name, thread_name(self())) == dave_false)
	{
		KVABNOR("The resource requested by %s is released by %s!",
			pKV->thread_name, thread_name(self()));
	}

	kv_timer_exit(pKV);

	kv_free_local(pKV);

	kv_free_remote(pKV, pKV->attrib);

	dave_memset(pKV, 0x00, sizeof(KV));

	kvm_free(pKV);
}

// ====================================================================

void
kv_struct_init(void)
{
	___kv_pv_booting___();
}

void
kv_struct_exit(void)
{

}

KV *
__kv_malloc__(dave_bool external_call, s8 *name, KVAttrib attrib, ub out_second, kv_out_callback callback_fun, s8 *fun, ub line)
{
	KV *pKV = NULL;

	___kv_pv_booting___();

	if(external_call == dave_true)
	{
		SAFEZONEv5W(_kv_struct_global_pv, pKV = ___kv_malloc___(name, attrib, out_second, callback_fun, fun, line););
	}
	else
	{
		pKV = ___kv_malloc___(name, attrib, out_second, callback_fun, fun, line);
	}

	return pKV;
}

void
kv_free(dave_bool external_call, KV *pKV)
{
	___kv_pv_booting___();

	if(external_call == dave_true)
	{
		SAFEZONEv5W(_kv_struct_global_pv, ___kv_free___(pKV););
	}
	else
	{
		___kv_free___(pKV);
	}
}

void
kv_timer(TIMERID timer_id, ub thread_index, void *param)
{
	KV *pKV = param;

	/*
	 * 时间事件会在线程空间排队，如果排队的过程中KV已经被释放了，
	 * 那么这个时候param携带的pKV指针就无效了，它会被其他模块使用。
	 * 这个时候如果使用param是一个危险行为。
	 */
	SAFEZONEv5W(_kv_struct_global_pv, {

		if(kv_check(pKV) == dave_true)
		{
			kv_timer_out(pKV);
		}

	} );
}

dave_bool
__kv_check__(KV *pKV, s8 *fun, ub line)
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

