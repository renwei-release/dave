/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#if defined(__DAVE_CYGWIN__) || defined(__DAVE_LINUX__)
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#ifdef __DAVE_LINUX__
#include <sys/syscall.h>
#endif
#ifdef __DAVE_CYGWIN__
#include "processthreadsapi.h"
#endif
#include "dave_linux.h"
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "os_log.h"

#define DAVE_THREAD_MAX (32)

typedef enum {
	THREADSTATE_INIT,
	THREADSTATE_RUNNING,
	THREADSTATE_STOPPED,
	THREADSTATE_CANCELED,
	THREADSTATE_RELEASED,
	THREADSTATE_SLEEP,
	THREADSTATE_MAX,
} THREADSTATE;

typedef struct {
	dave_os_thread_fun fun;
	void *arg;
	pthread_t thr_id;
	pthread_mutex_t m_mutex_t;
	pthread_cond_t  m_cond_t;
	volatile dave_bool signal_recd;
	volatile THREADSTATE state;
	s8 thread_name[15];
	ThreadId dave_thread_id;
} DAVEPTHREAD;

int pthread_setname_np(pthread_t thread, const char *name);

static DAVEPTHREAD _dave_pthread[DAVE_THREAD_MAX];
static pthread_spinlock_t _pv_lock;

static void
_thread_empty_handler(int signum)
{

}

static void
_thread_block_all_signal(void)
{
	sigset_t set;

	sigfillset(&set);

	pthread_sigmask(SIG_BLOCK, &set, NULL);
}

static void
_thread_set_break_signal(void)
{
	sigset_t set;
	struct sigaction act;

	sigemptyset(&set);
	sigaddset(&set, BREAK_SIG);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);

	act.sa_handler = _thread_empty_handler;
	sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
	//used to break blocked system calls
	sigaction(BREAK_SIG, &act, NULL);
}

static void *
_thread_function(void *arg)
{
	DAVEPTHREAD *id = (DAVEPTHREAD *)arg;

	_thread_block_all_signal();
	_thread_set_break_signal();

	pthread_mutex_lock(&(id->m_mutex_t));
	if (id->state != THREADSTATE_INIT)
	{
		pthread_mutex_unlock(&(id->m_mutex_t));
		return NULL;
	}
	else
	{
		id->state = THREADSTATE_RUNNING;
		pthread_mutex_unlock(&(id->m_mutex_t));
	}

	if ((id != NULL) && (id->fun))
	{
		pthread_setname_np(id->thr_id, (const char *)(id->thread_name));

		id->fun(id->arg);
	}

	return NULL;
}

static void
_thread_cancel(void *thread_id)
{
	DAVEPTHREAD *id = (DAVEPTHREAD *)thread_id;

	if(id != NULL)
	{
		pthread_mutex_lock(&(id->m_mutex_t));
		if(id->state == THREADSTATE_STOPPED)
		{
			pthread_mutex_unlock(&(id->m_mutex_t));
			return;
		}
		else
		{
			id->state = THREADSTATE_CANCELED;
			pthread_mutex_unlock(&(id->m_mutex_t));
		}

		pthread_kill(id->thr_id, BREAK_SIG);

		dave_os_thread_wakeup(thread_id);

		pthread_mutex_lock(&(id->m_mutex_t));
		while(id->state != THREADSTATE_STOPPED)
		{
			pthread_cond_wait(&(id->m_cond_t), &(id->m_mutex_t));
		}
		pthread_mutex_unlock(&(id->m_mutex_t));
	}
}

// =====================================================================

void
dave_os_init_thread(void)
{
	ub thread_index;

	for(thread_index=0; thread_index<DAVE_THREAD_MAX; thread_index++)
	{
		dave_memset(&_dave_pthread[thread_index], 0x00, sizeof(DAVEPTHREAD));

		_dave_pthread[thread_index].thr_id = (pthread_t)NULL;
	}

	pthread_spin_init(&_pv_lock, 0);

	signal(SIGPIPE, SIG_IGN);

	_thread_block_all_signal();
}

void
dave_os_exit_thread(void)
{
	pthread_spin_destroy(&_pv_lock);
}

void *
dave_os_create_thread(char *name, dave_os_thread_fun fun, void *arg)
{
	ub thread_index;
	pthread_attr_t process_attr;

	dave_os_pv_lock();
	for(thread_index=0; thread_index<DAVE_THREAD_MAX; thread_index++)
	{
		if(_dave_pthread[thread_index].fun == NULL)
		{
			_dave_pthread[thread_index].fun = fun;
			break;
		}
	}
	dave_os_pv_unlock(0);

	if(thread_index >= DAVE_THREAD_MAX)
	{
		OSABNOR("Please increase thread(%d) resources!", DAVE_THREAD_MAX);
		return NULL;
	}

	_dave_pthread[thread_index].arg = arg;
	pthread_mutex_init(&(_dave_pthread[thread_index].m_mutex_t), NULL);
	pthread_cond_init(&(_dave_pthread[thread_index].m_cond_t), NULL);
	_dave_pthread[thread_index].signal_recd = dave_false;
	_dave_pthread[thread_index].state = THREADSTATE_INIT;
	dave_strcpy(_dave_pthread[thread_index].thread_name, (s8 *)name, 15);
	_dave_pthread[thread_index].dave_thread_id = get_self();

	/* initialized with default attributes */
	pthread_attr_init(&process_attr);
	pthread_attr_setscope(&process_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&process_attr, PTHREAD_CREATE_DETACHED);

	if(pthread_create(&(_dave_pthread[thread_index].thr_id), &process_attr, _thread_function, (void *)(&(_dave_pthread[thread_index]))) != 0)
	{
		OSABNOR("Creat thread fail!");
		return NULL;
	}

	return (void *)(&(_dave_pthread[thread_index]));
}

void
dave_os_release_thread(void *thread_id)
{
	DAVEPTHREAD *id = (DAVEPTHREAD *)thread_id;

	if(id != NULL)
	{
		dave_os_thread_exit(thread_id);
	
		_thread_cancel(thread_id);

		/*
		系统退出的时候没有必要把锁退出，
		以免发生还有其他未尽事宜需要锁。	
		pthread_mutex_destroy(&(id->m_mutex_t));
		pthread_cond_destroy(&(id->m_cond_t));
		*/
		id->fun = NULL;
		id->state = THREADSTATE_RELEASED;
		id->thr_id = (pthread_t)NULL;
	}
}

void *
dave_os_thread_self(u64 *dave_thread_id)
{
	pthread_t thr_id;
	ub thread_index;

	if(dave_thread_id != NULL)
	{
		*dave_thread_id = INVALID_THREAD_ID;
	}

	thr_id = pthread_self();

	for(thread_index=0; thread_index<DAVE_THREAD_MAX; thread_index++)
	{
		if(_dave_pthread[thread_index].thr_id == thr_id)
		{
			if(dave_thread_id != NULL)
			{
				*dave_thread_id = _dave_pthread[thread_index].dave_thread_id;
			}

			return (void *)(&(_dave_pthread[thread_index]));
		}
	}

	return NULL;
}

ub
dave_os_thread_id(void)
{
#if defined(__DAVE_LINUX__)
	return (ub)syscall( __NR_gettid );
#elif defined(__DAVE_CYGWIN__)
	return (ub)GetCurrentThreadId();
#endif
}

dave_bool
dave_os_thread_sleep(void *thread_id)
{
	DAVEPTHREAD *id = (DAVEPTHREAD *)thread_id;

	if(id != NULL)
	{
		if(dave_os_thread_canceled(thread_id) == dave_false)
		{
			// child thread sleep.
			pthread_mutex_lock(&(id->m_mutex_t));
			while (id->signal_recd == dave_false)
			{
				// child thread sleep.
				pthread_cond_wait(&(id->m_cond_t), &(id->m_mutex_t));
			}
			id->signal_recd = dave_false;
			pthread_mutex_unlock(&(id->m_mutex_t));
		}
	}

	return dave_true;
}

dave_bool
dave_os_thread_wakeup(void *thread_id)
{
	DAVEPTHREAD *id = (DAVEPTHREAD *)thread_id;

	if(id != NULL)
	{
		pthread_mutex_lock(&(id->m_mutex_t));
		id->signal_recd = dave_true;
		pthread_mutex_unlock(&(id->m_mutex_t));
		// child thread wakeup.
		pthread_cond_broadcast(&(id->m_cond_t));
	}

	return dave_true;
}

dave_bool
dave_os_thread_canceled(void *thread_id)
{
	DAVEPTHREAD *id = (DAVEPTHREAD *)thread_id;

	if((id != NULL)
		&& ((id->state == THREADSTATE_STOPPED) || (id->state == THREADSTATE_CANCELED) || (id->state == THREADSTATE_RELEASED)))
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

void
dave_os_thread_exit(void *thread_id)
{
	DAVEPTHREAD *id = (DAVEPTHREAD *)thread_id;

	if (id != NULL)
	{
		pthread_mutex_lock(&(id->m_mutex_t));
		id->state = THREADSTATE_STOPPED;
		pthread_mutex_unlock(&(id->m_mutex_t));
		pthread_cond_broadcast(&(id->m_cond_t));
	}
}

sb
dave_os_pv_lock(void)
{
	pthread_spin_lock(&_pv_lock);

	return 0;
}

void
dave_os_pv_unlock(sb flag)
{
	pthread_spin_unlock(&_pv_lock);
}

void
dave_os_mutex_init(void *ptr)
{
	pthread_mutex_init((pthread_mutex_t *)ptr, NULL);
}

void
dave_os_mutex_destroy(void *ptr)
{
	pthread_mutex_destroy((pthread_mutex_t *)ptr);
}

void
dave_os_mutex_lock(void *ptr)
{
	pthread_mutex_lock((pthread_mutex_t *)ptr);
}

void
dave_os_mutex_unlock(void *ptr)
{
	pthread_mutex_unlock((pthread_mutex_t *)ptr);
}

dave_bool
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

void
dave_os_spin_lock_init(void *ptr)
{
	pthread_spin_init((pthread_spinlock_t *)ptr, 0);
}

void
dave_os_spin_lock_destroy(void *ptr)
{
	pthread_spin_destroy((pthread_spinlock_t *)ptr);
}

void
dave_os_spin_lock(void *ptr)
{
	pthread_spin_lock((pthread_spinlock_t *)ptr);
}

void
dave_os_spin_unlock(void *ptr)
{
	pthread_spin_unlock((pthread_spinlock_t *)ptr);
}

void
dave_os_spin_trylock(void *ptr)
{
	pthread_spin_trylock((pthread_spinlock_t *)ptr);
}

void
dave_os_rw_lock_init(void *ptr)
{
	pthread_rwlock_init((pthread_rwlock_t *)ptr, NULL);
}

void
dave_os_rw_lock_destroy(void *ptr)
{
	pthread_rwlock_destroy((pthread_rwlock_t *)ptr);
}

dave_bool
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

dave_bool
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

dave_bool
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

dave_bool
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

dave_bool
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

