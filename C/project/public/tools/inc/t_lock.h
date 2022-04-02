/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_LOCK_H__
#define __T_LOCK_H__
#include <pthread.h>
#include "tools_macro.h"
#include "dave_base.h"
#include "dave_tools.h"

typedef struct {
#ifdef __TOOLS_ALPHA_VERSION__
	ub magic_data_1;
#endif

	s8 spin_lock[sizeof(pthread_spinlock_t)];
	s8 rw_lock[sizeof(pthread_rwlock_t)];
	pthread_mutex_t m_mutex_t;

#ifdef __TOOLS_ALPHA_VERSION__
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

#define SAFEZONEv5R(pv, safe_zone){\
	t_rlock_rw(&pv);\
	{ safe_zone; }\
	t_unlock_rw(&pv);\
}

#define SAFEZONEv5W(pv, safe_zone){\
	t_wlock_rw(&pv);\
	{ safe_zone; }\
	t_unlock_rw(&pv);\
}

#define SAFEZONEv5TR(pv, safe_zone){\
	if(t_trlock_rw(&pv) == dave_true){\
		{ safe_zone; }\
		t_unlock_rw(&pv);\
	}\
}

#define SAFEZONEv5TW(pv, safe_zone){\
	if(t_twlock_rw(&pv)==dave_true){\
		{ safe_zone; }\
		t_unlock_rw(&pv);\
	}\
}

#ifdef __TOOLS_ALPHA_VERSION__

#define LOCK_ALARM_TIME_MS (1000000)

#define SAFEZONEv4(pv, safe_zone){\
	ub start_time = dave_os_time_ns();\
	t_lock_spin(&pv);\
	{ safe_zone; }\
	t_unlock_spin(&pv);\
	ub end_time = dave_os_time_ns();\
	if ((end_time-start_time) > LOCK_ALARM_TIME_MS){\
		DAVELOG("locked takes too long! <%s:%d/time:%ldms>\n", __func__, __LINE__, (end_time-start_time)/LOCK_ALARM_TIME_MS);\
	}\
}

#else

#define SAFEZONEv4(pv, safe_zone){\
	t_lock_spin(&pv);\
	{ safe_zone; }\
	t_unlock_spin(&pv);\
}

#endif
	
#define SAFEZONEv3(pv, safe_zone){\
	t_lock_mutex(&pv);\
	{ safe_zone; }\
	t_unlock_mutex(&pv);\
}

#define SAFEZONEidlev3(pv, safe_zone){\
	if(t_trylock_mutex(&pv) == dave_true){\
		{ safe_zone; }\
		t_unlock_mutex(&pv);\
	}\
}

#define SAFEZONEopenv3(pv, safe_zone){\
	t_lock_mutex(&pv);\
	{ safe_zone; }\
}

#define SAFEZONEclosev3(pv, safe_zone) { { safe_zone; }  t_unlock_mutex(&pv); }

#define SAFEZONEMutexLock SAFEZONEv3
#define SAFEZONESpinLock SAFEZONEv4
#define SAFEZONEReadLock SAFEZONEv5R
#define SAFEZONEWriteLock SAFEZONEv5W


// $$$$$$$$$$$$$$$$$$interface compatible$$$$$$$$$$$$$$$$$$$$
#define DaveLock TLock
#define dave_lock_reset t_lock_reset
#define dave_lock_destroy t_lock_destroy
#define dave_spin_lock t_lock_spin
#define dave_spin_unlock t_unlock_spin
#define dave_rw_rlock t_rlock_rw
#define dave_rw_wlock t_wlock_rw
#define dave_rw_trlock t_trlock_rw
#define dave_rw_twlock t_twlock_rw
#define dave_rw_unlock t_unlock_rw
#define dave_mutex_lock t_lock_mutex
#define dave_mutex_trylock t_trylock_mutex
#define dave_mutex_unlock t_unlock_mutex


#endif

