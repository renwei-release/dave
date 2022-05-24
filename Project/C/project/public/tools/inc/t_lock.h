/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_LOCK_H__
#define __T_LOCK_H__
#include <pthread.h>
#include "dave_base.h"
#include "dave_tools.h"

typedef struct {
#ifdef LEVEL_PRODUCT_alpha
	ub magic_data_1;
#endif

	s8 spin_lock[sizeof(pthread_spinlock_t)];
	s8 rw_lock[sizeof(pthread_rwlock_t)];
	pthread_mutex_t m_mutex_t;

#ifdef LEVEL_PRODUCT_alpha
	pthread_t thread_id;
	ub sleep;
	s8 *file;
	ub line;
	ub magic_data_2;
#endif
} TLock;

void t_lock_booting(void);
void t_lock_reset(TLock *pLock);
void t_lock_destroy(TLock *pLock);

void __t_lock_spin__(TLock *pLock, s8 *fun, ub line);
void __t_unlock_spin__(TLock *pLock, s8 *fun, ub line);
dave_bool __t_rlock_rw__(TLock *pLock, s8 *fun, ub line);
dave_bool __t_wlock_rw__(TLock *pLock, s8 *fun, ub line);
dave_bool __t_trlock_rw__(TLock *pLock, s8 *fun, ub line);
dave_bool __t_twlock_rw__(TLock *pLock, s8 *fun, ub line);
dave_bool __t_unlock_rw__(TLock *pLock, s8 *fun, ub line);
void __t_lock_mutex__(TLock *pLock, s8 *fun, ub line);
dave_bool __t_trylock_mutex__(TLock *pLock, s8 *fun, ub line);
void __t_unlock_mutex__(TLock *pLock, s8 *fun, ub line);

#define t_lock_spin(pLock) __t_lock_spin__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_unlock_spin(pLock) __t_unlock_spin__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_rlock_rw(pLock) __t_rlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_wlock_rw(pLock) __t_wlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_trlock_rw(pLock) __t_trlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_twlock_rw(pLock) __t_twlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_unlock_rw(pLock) __t_unlock_rw__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_lock_mutex(pLock) __t_lock_mutex__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_trylock_mutex(pLock) __t_trylock_mutex__(pLock, (s8 *)__func__, (ub)__LINE__)
#define t_unlock_mutex(pLock) __t_unlock_mutex__(pLock, (s8 *)__func__, (ub)__LINE__)

#define SAFECODEv2R(pv, safe_zone){\
	t_rlock_rw(&pv);\
	{ safe_zone; }\
	t_unlock_rw(&pv);\
}

#define SAFECODEv2W(pv, safe_zone){\
	t_wlock_rw(&pv);\
	{ safe_zone; }\
	t_unlock_rw(&pv);\
}

#define SAFECODEv2TR(pv, safe_zone){\
	if(t_trlock_rw(&pv) == dave_true){\
		{ safe_zone; }\
		t_unlock_rw(&pv);\
	}\
}

#define SAFECODEv2TW(pv, safe_zone){\
	if(t_twlock_rw(&pv)==dave_true){\
		{ safe_zone; }\
		t_unlock_rw(&pv);\
	}\
}
	
#define SAFECODEv1(pv, safe_zone){\
	t_lock_mutex(&pv);\
	{ safe_zone; }\
	t_unlock_mutex(&pv);\
}

#define SAFECODEidlev1(pv, safe_zone){\
	if(t_trylock_mutex(&pv) == dave_true){\
		{ safe_zone; }\
		t_unlock_mutex(&pv);\
	}\
}

#define SAFECODEopenv1(pv, safe_zone){\
	t_lock_mutex(&pv);\
	{ safe_zone; }\
}

#define SAFECODEclosev1(pv, safe_zone) { { safe_zone; }  t_unlock_mutex(&pv); }

#endif

