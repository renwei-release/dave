/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include <pthread.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "dave_verno.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "thread_tools.h"
#include "thread_log.h"

int pthread_setname_np(pthread_t thread, const char *name);

#define PROTECTOR_LIFE_MAX 12

typedef struct {
	sb life;
	s8 thread_name[THREAD_NAME_MAX];

	ThreadId thread_id;
	pthread_t thr_id;
} ThreadProtector;

static void
_thread_protector_read_wakeup(MSGBODY *msg)
{
	ThreadProtector *pProtector = (ThreadProtector *)(msg->user_ptr);

	pProtector->life = PROTECTOR_LIFE_MAX;
}

static void
_thread_protector_write_wakeup(ThreadProtector *pProtector)
{
	ProtectorWakeup *pWakeup = thread_msg(pWakeup);

	snd_from_msg(pProtector->thread_id, pProtector->thread_id, MSGID_PROTECTOR_WAKEUP, sizeof(ProtectorWakeup), pWakeup);
}

static void
_thread_protector_behavior(ThreadProtector *pProtector)
{
	fprintf(stdout, "****** The protector discovered that the protected:%s was unresponsive. ******\n", pProtector->thread_name);
	fprintf(stdout, "****** Now intentionally generate a coredump file, ******\n");
	fprintf(stdout, "****** The system will then restart. ******\n");

	// coredump !!!!!!
	int* p = NULL;
	*p = 0;

	fprintf(stdout, "****** If coredump does not cause the system to restart, then exit the system. ******\n");
	exit(0);
}

static void
_thread_protector_setup_name(ThreadProtector *pProtector)
{
	pthread_setname_np(pProtector->thr_id, (const char *)(pProtector->thread_name));
}

static void *
_thread_protector_function(void *arg)
{
	ThreadProtector *pProtector = (ThreadProtector *)arg;

	_thread_protector_setup_name(pProtector);

	THREADLOG("The protector of %s ready", pProtector->thread_name);

	while(base_power_state() == dave_true)
	{
		pProtector->life --;

		_thread_protector_write_wakeup(pProtector);

		dave_os_sleep(6000);

		if(pProtector->life <= 0)
		{
			_thread_protector_behavior(pProtector);
		}
	}

	THREADLOG("The protector of %s exit", pProtector->thread_name);

	dave_free(pProtector);

	return NULL;
}

static dave_bool
_thread_protector_creat_thread(ThreadProtector *pProtector)
{
	pthread_attr_t process_attr;

	pthread_attr_init(&process_attr);
	pthread_attr_setscope(&process_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&process_attr, PTHREAD_CREATE_DETACHED);

	if(pthread_create(&(pProtector->thr_id), &process_attr, _thread_protector_function, (void *)pProtector) != 0)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

// =====================================================================

void
thread_protector_reg(ThreadId thread_id)
{
	s8 *thread_name = thread_id_to_name(thread_id);

	ThreadProtector *pProtector = dave_ralloc(sizeof(ThreadProtector));

	pProtector->life = PROTECTOR_LIFE_MAX;

	pProtector->thread_id = thread_id;
	dave_strcpy(pProtector->thread_name, thread_name, sizeof(pProtector->thread_name));

	base_thread_msg_register(thread_id, (ub)MSGID_PROTECTOR_WAKEUP, _thread_protector_read_wakeup, pProtector);

	_thread_protector_creat_thread(pProtector);
}

void
thread_protector_unreg(ThreadId thread_id)
{
	THREADLOG("unsupport unreg:%s", thread_id_to_name(thread_id));
}

#endif

