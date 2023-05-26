/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OS_THREAD_H__
#define __DAVE_OS_THREAD_H__
#include <pthread.h>

typedef void *(* dave_os_thread_fun)(void *arg);

void dave_os_init_thread(void);

void dave_os_exit_thread(void);

void *dave_os_create_thread(char *name, dave_os_thread_fun fun, void *arg);

void dave_os_release_thread(void *thread_id);

void *dave_os_thread_self(u64 *dave_thread_id);

ub dave_os_thread_id(void);

dave_bool dave_os_thread_sleep(void *thread_id);

dave_bool dave_os_thread_wakeup(void *thread_id);

dave_bool dave_os_thread_canceled(void *thread_id);

static inline void
dave_os_mutex_init(void *ptr)
{
	pthread_mutex_init((pthread_mutex_t *)ptr, NULL);
}

static inline void
dave_os_mutex_destroy(void *ptr)
{
	pthread_mutex_destroy((pthread_mutex_t *)ptr);
}

static inline void
dave_os_mutex_lock(void *ptr)
{
	pthread_mutex_lock((pthread_mutex_t *)ptr);
}

static inline void
dave_os_mutex_unlock(void *ptr)
{
	pthread_mutex_unlock((pthread_mutex_t *)ptr);
}

static inline dave_bool
dave_os_mutex_trylock(void *ptr)
{
	if(pthread_mutex_trylock((pthread_mutex_t *)ptr) == 0)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

static inline void
dave_os_spin_lock_init(void *ptr)
{
	pthread_spin_init((pthread_spinlock_t *)ptr, 0);
}

static inline void
dave_os_spin_lock_destroy(void *ptr)
{
	pthread_spin_destroy((pthread_spinlock_t *)ptr);
}

static inline void
dave_os_spin_lock(void *ptr)
{
	pthread_spin_lock((pthread_spinlock_t *)ptr);
}

static inline void
dave_os_spin_unlock(void *ptr)
{
	pthread_spin_unlock((pthread_spinlock_t *)ptr);
}

static inline void
dave_os_spin_trylock(void *ptr)
{
	pthread_spin_trylock((pthread_spinlock_t *)ptr);
}

static inline void
dave_os_rw_lock_init(void *ptr)
{
	pthread_rwlock_init((pthread_rwlock_t *)ptr, NULL);
}

static inline void
dave_os_rw_lock_destroy(void *ptr)
{
	pthread_rwlock_destroy((pthread_rwlock_t *)ptr);
}

static inline dave_bool
dave_os_rw_rlock(void *ptr)
{
	if(pthread_rwlock_rdlock((pthread_rwlock_t *)ptr) == 0)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

static inline dave_bool
dave_os_rw_wlock(void *ptr)
{
	if(pthread_rwlock_wrlock((pthread_rwlock_t *)ptr) == 0)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

static inline dave_bool
dave_os_rw_tryrlock(void *ptr)
{
	if(pthread_rwlock_tryrdlock((pthread_rwlock_t *)ptr) == 0)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

static inline dave_bool
dave_os_rw_trywlock(void *ptr)
{
	if(pthread_rwlock_trywrlock((pthread_rwlock_t *)ptr) == 0)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

static inline dave_bool
dave_os_rw_unlock(void *ptr)
{
	if(pthread_rwlock_unlock((pthread_rwlock_t *)ptr) == 0)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

#endif

