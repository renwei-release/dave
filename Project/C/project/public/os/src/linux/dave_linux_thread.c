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
#include <stdint.h>
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
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "os_log.h"

#define DAVE_THREAD_MAX (32)
#define THREAD_HANDLE_BITS 6
#define THREAD_HANDLE_MASK (((uintptr_t)1 << THREAD_HANDLE_BITS) - 1)

typedef enum {
	THREADSTATE_INIT,
	THREADSTATE_RUNNING,
	THREADSTATE_SLEEP,
	THREADSTATE_RELEASED,
	THREADSTATE_MAX,
} THREADSTATE;

typedef struct {
	dave_os_thread_fun fun;
	void *arg;
	pthread_t thr_id;
	pthread_mutex_t m_mutex_t;
	pthread_cond_t  m_cond_t;
	dave_bool signal_recd;
	THREADSTATE state;
	dave_bool initialized;
	uintptr_t generation;
	s8 thread_name[15];
	ThreadId dave_thread_id;
} DAVEPTHREAD;

int pthread_setname_np(pthread_t thread, const char *name);

static DAVEPTHREAD _dave_pthread[DAVE_THREAD_MAX];
static pthread_mutex_t _thread_table_mutex = PTHREAD_MUTEX_INITIALIZER;

// Opaque handles include a generation so a reused slot rejects old requests.
static void *
_thread_handle(DAVEPTHREAD *id)
{
	return (void *)((id->generation << THREAD_HANDLE_BITS)
		| (uintptr_t)(id - _dave_pthread + 1));
}

static DAVEPTHREAD *
_thread_find(void *handle)
{
	uintptr_t value = (uintptr_t)handle;
	uintptr_t slot = value & THREAD_HANDLE_MASK;
	DAVEPTHREAD *id;

	if((slot == 0) || (slot > DAVE_THREAD_MAX))
		return NULL;
	id = &_dave_pthread[slot - 1];
	if((id->fun == NULL) || (id->generation != (value >> THREAD_HANDLE_BITS)))
		return NULL;
	return id;
}

static void
_thread_recycle(void *arg)
{
	DAVEPTHREAD *id = arg;

	pthread_mutex_lock(&_thread_table_mutex);
	pthread_mutex_lock(&id->m_mutex_t);
	id->state = THREADSTATE_RELEASED;
	id->signal_recd = dave_false;
	id->fun = NULL;
	id->arg = NULL;
	pthread_mutex_unlock(&id->m_mutex_t);
	pthread_mutex_unlock(&_thread_table_mutex);
}

static void
_thread_sleep_unlock(void *arg)
{
	pthread_mutex_unlock(arg);
}

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
	dave_os_thread_fun fun = NULL;
	void *fun_arg = NULL;
	s8 name[sizeof(id->thread_name)];

	pthread_cleanup_push(_thread_recycle, id);
	_thread_block_all_signal();
	_thread_set_break_signal();

	pthread_mutex_lock(&_thread_table_mutex);
	pthread_mutex_lock(&(id->m_mutex_t));
	if(id->state == THREADSTATE_INIT)
	{
		id->state = THREADSTATE_RUNNING;
		fun = id->fun;
		fun_arg = id->arg;
		dave_memcpy(name, id->thread_name, sizeof(name));
	}
	pthread_mutex_unlock(&(id->m_mutex_t));
	pthread_mutex_unlock(&_thread_table_mutex);

	if(fun != NULL)
	{
		pthread_setname_np(pthread_self(), (const char *)name);
		fun(fun_arg);
	}
	pthread_cleanup_pop(1);

	return NULL;
}

// =====================================================================

void
dave_os_init_thread(void)
{
	signal(SIGPIPE, SIG_IGN);

	_thread_block_all_signal();
}

void
dave_os_exit_thread(void)
{

}

void *
dave_os_create_thread(char *name, dave_os_thread_fun fun, void *arg)
{
	ub thread_index;
	pthread_attr_t process_attr;
	DAVEPTHREAD *id;
	void *handle = NULL;
	ThreadId dave_thread_id;
	int rc, destroy_rc;

	if((name == NULL) || (fun == NULL))
	{
		OSABNOR("invalid thread name or function!");
		return NULL;
	}
	dave_thread_id = get_self();
	rc = pthread_attr_init(&process_attr);
	if(rc != 0)
	{
		OSABNOR("thread attribute init failed:%d<%s>!", rc, strerror(rc));
		return NULL;
	}
	rc = pthread_attr_setdetachstate(&process_attr, PTHREAD_CREATE_DETACHED);
	if(rc != 0)
	{
		destroy_rc = pthread_attr_destroy(&process_attr);
		if(destroy_rc != 0)
			OSABNOR("thread attribute destroy failed:%d<%s>!", destroy_rc, strerror(destroy_rc));
		OSABNOR("thread detach attribute failed:%d<%s>!", rc, strerror(rc));
		return NULL;
	}

	pthread_mutex_lock(&_thread_table_mutex);
	for(thread_index=0; thread_index<DAVE_THREAD_MAX; thread_index++)
	{
		if((_dave_pthread[thread_index].fun == NULL)
			&& (_dave_pthread[thread_index].generation < (UINTPTR_MAX >> THREAD_HANDLE_BITS)))
			break;
	}

	if(thread_index >= DAVE_THREAD_MAX)
	{
		pthread_mutex_unlock(&_thread_table_mutex);
		destroy_rc = pthread_attr_destroy(&process_attr);
		if(destroy_rc != 0)
			OSABNOR("thread attribute destroy failed:%d<%s>!", destroy_rc, strerror(destroy_rc));
		OSABNOR("Please increase thread(%d) resources!", DAVE_THREAD_MAX);
		return NULL;
	}

	id = &_dave_pthread[thread_index];
	if(id->initialized == dave_false)
	{
		rc = pthread_mutex_init(&id->m_mutex_t, NULL);
		if(rc == 0)
		{
			rc = pthread_cond_init(&id->m_cond_t, NULL);
			if(rc == 0)
				id->initialized = dave_true;
			else
			{
				destroy_rc = pthread_mutex_destroy(&id->m_mutex_t);
				if(destroy_rc != 0)
					rc = destroy_rc;
			}
		}
	}
	if(rc == 0)
	{
		id->generation ++;
		id->fun = fun;
		id->arg = arg;
		id->signal_recd = dave_false;
		id->state = THREADSTATE_INIT;
		dave_strcpy(id->thread_name, (s8 *)name, sizeof(id->thread_name));
		id->dave_thread_id = dave_thread_id;
		handle = _thread_handle(id);

		rc = pthread_create(&id->thr_id, &process_attr, _thread_function, id);
		if(rc != 0)
		{
			id->fun = NULL;
			id->arg = NULL;
			id->state = THREADSTATE_RELEASED;
			handle = NULL;
		}
	}
	pthread_mutex_unlock(&_thread_table_mutex);
	destroy_rc = pthread_attr_destroy(&process_attr);
	if(destroy_rc != 0)
		OSABNOR("thread attribute destroy failed:%d<%s>!", destroy_rc, strerror(destroy_rc));
	if(rc != 0)
		OSABNOR("thread create/init failed:%d<%s>!", rc, strerror(rc));

	return handle;
}

void
dave_os_release_thread(void *thread_id)
{
	DAVEPTHREAD *id;
	int rc = 0;

	pthread_mutex_lock(&_thread_table_mutex);
	id = _thread_find(thread_id);
	if(id != NULL)
	{
		pthread_mutex_lock(&id->m_mutex_t);
		id->state = THREADSTATE_RELEASED;
		id->signal_recd = dave_true;
		rc = pthread_cond_broadcast(&id->m_cond_t);
		pthread_mutex_unlock(&id->m_mutex_t);
	}
	pthread_mutex_unlock(&_thread_table_mutex);
	if(rc != 0)
		OSABNOR("thread release wakeup failed:%d<%s>!", rc, strerror(rc));
	if((id == NULL) && (thread_id != NULL))
		OSDEBUG("inactive thread handle:%p", thread_id);
}

void *
dave_os_thread_self(u64 *dave_thread_id)
{
	pthread_t thr_id;
	ub thread_index;
	void *handle = NULL;

	if(dave_thread_id != NULL)
	{
		*dave_thread_id = INVALID_THREAD_ID;
	}

	thr_id = pthread_self();

	pthread_mutex_lock(&_thread_table_mutex);
	for(thread_index=0; thread_index<DAVE_THREAD_MAX; thread_index++)
	{
		if((_dave_pthread[thread_index].fun != NULL)
			&& pthread_equal(_dave_pthread[thread_index].thr_id, thr_id))
		{
			if(dave_thread_id != NULL)
			{
				*dave_thread_id = _dave_pthread[thread_index].dave_thread_id;
			}

			handle = _thread_handle(&_dave_pthread[thread_index]);
			break;
		}
	}
	pthread_mutex_unlock(&_thread_table_mutex);

	return handle;
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
	DAVEPTHREAD *id;
	int rc = 0;

	if(thread_id == NULL)
		return dave_true;
	pthread_mutex_lock(&_thread_table_mutex);
	id = _thread_find(thread_id);
	if(id == NULL)
	{
		pthread_mutex_unlock(&_thread_table_mutex);
		OSDEBUG("inactive thread sleep handle:%p", thread_id);
		return dave_false;
	}
	if(!pthread_equal(id->thr_id, pthread_self()))
	{
		pthread_mutex_unlock(&_thread_table_mutex);
		OSLOG("thread sleep requires the calling thread's handle!");
		return dave_false;
	}
	pthread_mutex_lock(&id->m_mutex_t);
	pthread_mutex_unlock(&_thread_table_mutex);

	// Cancellation of cond_wait reacquires the mutex before cleanup runs.
	pthread_cleanup_push(_thread_sleep_unlock, &id->m_mutex_t);
	while((id->signal_recd == dave_false) && (id->state != THREADSTATE_RELEASED))
	{
		rc = pthread_cond_wait(&id->m_cond_t, &id->m_mutex_t);
		if(rc != 0)
			break;
	}
	id->signal_recd = dave_false;
	pthread_cleanup_pop(1);
	if(rc != 0)
		OSABNOR("thread sleep failed:%d<%s>!", rc, strerror(rc));
	return (rc == 0) ? dave_true : dave_false;
}

dave_bool
dave_os_thread_wakeup(void *thread_id)
{
	DAVEPTHREAD *id;
	int rc = 0;

	if(thread_id == NULL)
		return dave_true;
	pthread_mutex_lock(&_thread_table_mutex);
	id = _thread_find(thread_id);
	if(id != NULL)
	{
		pthread_mutex_lock(&id->m_mutex_t);
		id->signal_recd = dave_true;
		rc = pthread_cond_broadcast(&id->m_cond_t);
		pthread_mutex_unlock(&id->m_mutex_t);
	}
	pthread_mutex_unlock(&_thread_table_mutex);
	if(id == NULL)
		OSDEBUG("inactive thread wakeup handle:%p", thread_id);
	if(rc != 0)
		OSABNOR("thread wakeup failed:%d<%s>!", rc, strerror(rc));

	return ((id != NULL) && (rc == 0)) ? dave_true : dave_false;
}

dave_bool
dave_os_thread_canceled(void *thread_id)
{
	DAVEPTHREAD *id;
	dave_bool canceled = dave_false;

	if(thread_id != NULL)
	{
		pthread_mutex_lock(&_thread_table_mutex);
		id = _thread_find(thread_id);
		canceled = (id == NULL) || (id->state == THREADSTATE_RELEASED);
		pthread_mutex_unlock(&_thread_table_mutex);
		if(id == NULL)
			OSDEBUG("inactive thread cancel handle:%p", thread_id);
	}
	return canceled;
}

#endif

