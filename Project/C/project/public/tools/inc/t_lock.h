/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_LOCK_H__
#define __T_LOCK_H__
#include <pthread.h>
#include "dave_os_thread.h"
#include "tools_log.h"

typedef struct {
#ifdef LEVEL_PRODUCT_alpha
	ub magic_data_1;
	pthread_t thread_id;
	ub sleep;
	s8 *file;
	ub line;
	ub magic_data_2;
#endif

	s8 spin_lock[sizeof(pthread_spinlock_t)];
	s8 rw_lock[sizeof(pthread_rwlock_t)];
	pthread_mutex_t m_mutex_t;
} TLock;

extern TLock *_t_booting_lock;

void t_lock_booting(void);
void t_lock_reset(TLock *pLock);
void t_lock_destroy(TLock *pLock);

static inline void
__t_lock_spin__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSERROR("Found an invalid lock[%s:%ld, pLock:%lx]!",
			fun, line, (ub)pLock);
		return;
	}
#endif

	dave_os_spin_lock((void *)(pLock->spin_lock));
}

static inline void
__t_unlock_spin__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSERROR("Found an invalid lock[%s:%ld, pLock:%lx]!",
			fun, line, (ub)pLock);
		return;
	}
#endif

	dave_os_spin_unlock((void *)(pLock->spin_lock));
}

#ifdef LEVEL_PRODUCT_alpha

static inline dave_bool
_t_lock_check(TLock *pLock, s8 *fun, ub line)
{
	dave_bool ret;

	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSERROR("Found an invalid lock[%s:%ld, pLock:%lx]!",
			fun, line, (ub)pLock);
		return dave_false;
	}

	ret = dave_true;

	__t_lock_spin__(pLock, fun, line);
	if(pLock->thread_id == (pthread_t)-1)
	{
		pLock->thread_id = (pthread_t)pthread_self();
		pLock->file = fun;
		pLock->line = line;
	}
	else if(pLock->thread_id == (pthread_t)pthread_self())
	{
		TOOLSERROR("%s:%ld->%s:%ld a nested lock occurred or lock was not released correctly!",
			pLock->file, pLock->line, fun, line);
		ret = dave_false;
	}
	__t_unlock_spin__(pLock, fun, line);

	return ret;
}

static inline dave_bool
_t_unlock_check(TLock *pLock, s8 *fun, ub line)
{
	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSERROR("Found an invalid lock! [%s:%ld magic:%lx/%lx]",
			fun, line, pLock->magic_data_1, pLock->magic_data_2);
		return dave_false;
	}

	__t_lock_spin__(pLock, fun, line);
	if(pLock->thread_id == pthread_self())
	{
		pLock->thread_id = (pthread_t)-1;
		pLock->file = NULL;
		pLock->line = 0;
	}
	__t_unlock_spin__(pLock, fun, line);

	return dave_true;
}

static inline dave_bool
_t_lock_uncontested_check(TLock *pLock, s8 *fun, ub line)
{
	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSERROR("Found an invalid lock[%s:%ld, pLock:%lx]!",
			fun, line, (ub)pLock);
		return dave_false;
	}

	return dave_true;
}

static inline dave_bool
_t_unlock_uncontested_check(TLock *pLock, s8 *fun, ub line)
{
	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSERROR("Found an invalid lock! [%s:%ld magic:%lx/%lx]",
			fun, line, pLock->magic_data_1, pLock->magic_data_2);
		return dave_false;
	}

	return dave_true;
}

#endif

static inline dave_bool
__t_rlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(_t_lock_uncontested_check(pLock, fun, line) == dave_false)
		return dave_false;
#endif
	return dave_os_rw_rlock((void *)(pLock->rw_lock));
}

static inline dave_bool
__t_runlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(_t_unlock_uncontested_check(pLock, fun, line) == dave_false)
		return dave_false;
#endif
	return dave_os_rw_unlock((void *)(pLock->rw_lock));
}

static inline dave_bool
__t_wlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(_t_lock_check(pLock, fun, line) == dave_false)
		return dave_false;
#endif
	return dave_os_rw_wlock((void *)(pLock->rw_lock));
}

static inline dave_bool
__t_trlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	dave_bool ret;

	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(_t_lock_check(pLock, fun, line) == dave_true)
	{
#endif
		ret = dave_os_rw_tryrlock((void *)(pLock->rw_lock));
#ifdef LEVEL_PRODUCT_alpha
		if(ret == dave_false)
		{
			_t_unlock_check(pLock, fun, line);
		}
	}
	else
	{
		ret = dave_false;
	}
#endif

	return ret;
}

static inline dave_bool
__t_twlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	dave_bool ret;

	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(_t_lock_check(pLock, fun, line) == dave_true)
	{
#endif
		ret = dave_os_rw_trywlock((void *)(pLock->rw_lock));
#ifdef LEVEL_PRODUCT_alpha
		if(ret == dave_false)
		{
			_t_unlock_check(pLock, fun, line);
		}
	}
	else
	{
		ret = dave_false;
	}
#endif

	return ret;
}

static inline dave_bool
__t_unlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(_t_unlock_check(pLock, fun, line) == dave_false)
		return dave_false;
#endif
	return dave_os_rw_unlock((void *)(pLock->rw_lock));
}

static inline dave_bool
__t_lock_mutex__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(_t_lock_check(pLock, fun, line) == dave_false)
		return dave_false;
#endif
	dave_os_mutex_lock((void *)(&(pLock->m_mutex_t)));
	return dave_true;
}

static inline dave_bool
__t_trylock_mutex__(TLock *pLock, s8 *fun, ub line)
{
	dave_bool ret;

	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(_t_lock_uncontested_check(pLock, fun, line) == dave_true)
	{
#endif
		ret = dave_os_mutex_trylock((void *)(&(pLock->m_mutex_t)));
#ifdef LEVEL_PRODUCT_alpha
		if(ret == dave_false)
		{
			_t_unlock_uncontested_check(pLock, fun, line);
		}
	}
	else
	{
		ret = dave_false;
	}
#endif

	return ret;
}

static inline void
__t_unlock_mutex__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = _t_booting_lock;
	}

#ifdef LEVEL_PRODUCT_alpha
	if(_t_unlock_check(pLock, fun, line) == dave_true)
#endif
		dave_os_mutex_unlock((void *)(&(pLock->m_mutex_t)));
}

#define t_lock_spin(pLock) __t_lock_spin__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_unlock_spin(pLock) __t_unlock_spin__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_rlock_rw(pLock) __t_rlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_runlock_rw(pLock) __t_runlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_wlock_rw(pLock) __t_wlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_trlock_rw(pLock) __t_trlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_twlock_rw(pLock) __t_twlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_unlock_rw(pLock) __t_unlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_lock_mutex(pLock) __t_lock_mutex__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_trylock_mutex(pLock) __t_trylock_mutex__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_unlock_mutex(pLock) __t_unlock_mutex__(pLock, (s8 *)__func__, (ub)__LINE__)

#define t_lock t_lock_spin(NULL)
#define t_unlock t_unlock_spin(NULL)

#define ENABLE_SAFE_PRE_FLAG 0xaaff45290fdc

#define SAFEPre(flag, safe_zone) {\
	if(flag != ENABLE_SAFE_PRE_FLAG) {\
		t_lock;\
		if(flag != ENABLE_SAFE_PRE_FLAG) {\
			{ safe_zone; }\
			flag = ENABLE_SAFE_PRE_FLAG;\
		}\
		t_unlock;\
	}\
}

#define SAFECODEv2R(pv, safe_zone) {\
	if(t_rlock_rw(&pv) == dave_true) {\
		{ safe_zone; }\
		t_runlock_rw(&pv);\
	}\
}

#define SAFECODEv2W(pv, safe_zone) {\
	if(t_wlock_rw(&pv) == dave_true) {\
		{ safe_zone; }\
		t_unlock_rw(&pv);\
	}\
}

#define SAFECODEv2TR(pv, safe_zone) {\
	if(t_trlock_rw(&pv) == dave_true) {\
		{ safe_zone; }\
		t_unlock_rw(&pv);\
	}\
}

#define SAFECODEv2TW(pv, safe_zone) {\
	if(t_twlock_rw(&pv) == dave_true) {\
		{ safe_zone; }\
		t_unlock_rw(&pv);\
	}\
}
	
#define SAFECODEv1(pv, safe_zone) {\
	if(t_lock_mutex(&pv) == dave_true) {\
		{ safe_zone; }\
		t_unlock_mutex(&pv);\
	}\
}

#define SAFECODEidlev1(pv, safe_zone) {\
	if(t_trylock_mutex(&pv) == dave_true) {\
		{ safe_zone; }\
		t_unlock_mutex(&pv);\
	}\
}

#endif

