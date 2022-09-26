/*
* Tencent is pleased to support the open source community by making Libco available.

* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

#include "3rdparty_macro.h"
#ifdef COROUTINE_3RDPARTY
#include "co_routine.h"
#include "co_routine_inner.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>

#include <poll.h>
#include <sys/time.h>
#include <errno.h>

#include <assert.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __DAVE_LINUX__
#include <sys/syscall.h>
#endif
#ifdef __DAVE_CYGWIN__
#include "processthreadsapi.h"
#endif
#include <unistd.h>
#include <limits.h>

#include "dave_base.h"
#include "party_log.h"

#define CALL_STACK_MAX 128
#define TID_MAX DAVE_SYS_THREAD_ID_MAX

extern "C"
{
	extern void coctx_swap( coctx_t *, coctx_t* ) asm("coctx_swap");
};
using namespace std;
stCoRoutine_t *GetCurrCo( stCoRoutineEnv_t *env );

struct stCoRoutineEnv_t
{
	stCoRoutine_t *pCallStack[ CALL_STACK_MAX ];
	int iCallStackSize;

	//for copy stack log lastco and nextco
	stCoRoutine_t* pending_co;
	stCoRoutine_t* occupy_co;
};

void co_log_err( const char *fmt,... )
{
	PARTYLOG(fmt);
}

static pid_t GetPid()
{
    static __thread pid_t pid = 0;
    static __thread pid_t tid = 0;

    if( !pid || !tid || pid != getpid() )
    {
        pid = getpid();
#if defined( __APPLE__ )
		tid = syscall( SYS_gettid );
		if( -1 == (long)tid )
		{
			tid = pid;
		}
#elif defined( __FreeBSD__ )
		syscall(SYS_thr_self, &tid);
		if( tid < 0 )
		{
			tid = pid;
		}
#elif defined(__DAVE_LINUX__) 
        tid = syscall( __NR_gettid );
#elif defined(__DAVE_CYGWIN__)
		tid = (pid_t)GetCurrentThreadId();
#else
#error Please define platform!
#endif
    }

    return tid % TID_MAX;
}

/////////////////for copy stack //////////////////////////
stStackMem_t* co_alloc_stackmem(unsigned int stack_size)
{
	stStackMem_t* stack_mem = (stStackMem_t*)dave_malloc(sizeof(stStackMem_t));
	stack_mem->occupy_co= NULL;
	stack_mem->stack_size = stack_size;
	stack_mem->stack_buffer = (char*)dave_malloc(stack_size);
	stack_mem->stack_bp = stack_mem->stack_buffer + stack_size;
	return stack_mem;
}

void co_free_stackmem(stStackMem_t *stack_mem)
{
	if(stack_mem != NULL)
	{
		dave_free(stack_mem->stack_buffer);

		dave_free(stack_mem);
	}
}

stShareStack_t* co_alloc_sharestack(int count, int stack_size)
{
	stShareStack_t* share_stack = (stShareStack_t*)dave_malloc(sizeof(stShareStack_t));
	share_stack->alloc_idx = 0;
	share_stack->stack_size = stack_size;

	//alloc stack array
	share_stack->count = count;
	stStackMem_t** stack_array = (stStackMem_t**)dave_ralloc(count * sizeof(stStackMem_t*));
	for (int i = 0; i < count; i++)
	{
		stack_array[i] = co_alloc_stackmem(stack_size);
	}
	share_stack->stack_array = stack_array;
	return share_stack;
}

void co_free_sharestack(stShareStack_t *share_stack)
{
	int index;

	for(index=0; index<share_stack->count; index++)
	{
		co_free_stackmem(share_stack->stack_array[index]);
	}
	dave_free(share_stack->stack_array);

	dave_free(share_stack);
}

static stStackMem_t* co_get_stackmem(stShareStack_t* share_stack)
{
	if (!share_stack)
	{
		return NULL;
	}
	int idx = share_stack->alloc_idx % share_stack->count;
	share_stack->alloc_idx++;

	return share_stack->stack_array[idx];
}

static int CoRoutineFunc( stCoRoutine_t *co,void * )
{
	if( co->pfn )
	{
		co->pfn( co->arg );
	}
	co->cEnd = 1;

	stCoRoutineEnv_t *env = co->env;

	co_yield_env( env );

	return 0;
}

struct stCoRoutine_t *co_create_env( stCoRoutineEnv_t * env, const stCoRoutineAttr_t *attr,
		pfn_co_routine_t pfn,void *arg )
{
	stCoRoutineAttr_t at;

	if( attr )
	{
		memcpy( &at, attr, sizeof(at) );
	}
	if( at.stack_size <= 0 )
	{
		at.stack_size = STACKSIZE;
	}
	else if( at.stack_size > 1024 * 1024 * 8 )
	{
		at.stack_size = 1024 * 1024 * 8;
	}

	if( at.stack_size & 0xFFF ) 
	{
		at.stack_size &= ~0xFFF;
		at.stack_size += 0x1000;
	}

	stCoRoutine_t *lp = (stCoRoutine_t*)dave_ralloc( sizeof(stCoRoutine_t) );

	lp->env = env;
	lp->pfn = pfn;
	lp->arg = arg;

	stStackMem_t* stack_mem = NULL;
	if( at.share_stack )
	{
		stack_mem = co_get_stackmem( at.share_stack);
		at.stack_size = at.share_stack->stack_size;
	}
	else
	{
		stack_mem = co_alloc_stackmem(at.stack_size);
	}
	lp->stack_mem = stack_mem;

	lp->ctx.ss_sp = stack_mem->stack_buffer;
	lp->ctx.ss_size = at.stack_size;

	lp->cStart = 0;
	lp->cEnd = 0;
	lp->cIsMain = 0;
	lp->cIsShareStack = at.share_stack != NULL;

	lp->save_size = 0;
	lp->save_buffer = NULL;

	return lp;
}

extern "C" int
co_create( stCoRoutine_t **ppco, pfn_co_routine_t pfn, void *arg )
{
	stCoRoutineAttr_t attr;

	if( !co_get_curr_thread_env() ) 
	{
		co_init_curr_thread_env();
	}

	attr.share_stack = co_alloc_sharestack(1, attr.stack_size);

	stCoRoutine_t *co = co_create_env( co_get_curr_thread_env(), &attr, pfn, arg );

	co->user_attr = attr;

	*ppco = co;

	return 0;
}

extern "C" void
co_free( stCoRoutine_t *co )
{
    if (!co->cIsShareStack) 
    {
        dave_free(co->stack_mem->stack_buffer);
        dave_free(co->stack_mem);
    }
	else
	{
		co_free_sharestack(co->user_attr.share_stack);
	}
    dave_free( co );
}

extern "C" void
co_release( stCoRoutine_t *co )
{
    co_free( co );
}

void co_swap(stCoRoutine_t* curr, stCoRoutine_t* pending_co);

extern "C" void
co_resume( stCoRoutine_t *co )
{
	stCoRoutineEnv_t *env = co->env;
	stCoRoutine_t *lpCurrRoutine = env->pCallStack[ env->iCallStackSize - 1 ];
	if( !co->cStart )
	{
		coctx_make( &co->ctx, (coctx_pfn_t)CoRoutineFunc, co, 0 );
		co->cStart = 1;
	}
	if(env->iCallStackSize >= CALL_STACK_MAX)
	{
		co_log_err("invalid env->iCallStackSize:%d", env->iCallStackSize);
		return;
	}
	env->pCallStack[ env->iCallStackSize ++ ] = co;
	co_swap( lpCurrRoutine, co );
}

extern "C" void
co_yield_env( stCoRoutineEnv_t *env )
{
	stCoRoutine_t *last = env->pCallStack[ env->iCallStackSize - 2 ];
	stCoRoutine_t *curr = env->pCallStack[ env->iCallStackSize - 1 ];

	env->iCallStackSize --;

	co_swap( curr, last);
}

extern "C" void
co_yield_ct()
{
	co_yield_env( co_get_curr_thread_env() );
}

extern "C" void
co_yield( stCoRoutine_t *co )
{
	co_yield_env( co->env );
}

void save_stack_buffer(stCoRoutine_t* occupy_co)
{
	///copy out
	stStackMem_t* stack_mem = occupy_co->stack_mem;
	int len = stack_mem->stack_bp - occupy_co->stack_sp;

	if (occupy_co->save_buffer)
	{
		dave_free(occupy_co->save_buffer), occupy_co->save_buffer = NULL;
	}

	occupy_co->save_buffer = (char*)dave_malloc(len); //malloc buf;
	occupy_co->save_size = len;

	memcpy(occupy_co->save_buffer, occupy_co->stack_sp, len);
}

void co_swap(stCoRoutine_t* curr, stCoRoutine_t* pending_co)
{
 	stCoRoutineEnv_t *env = co_get_curr_thread_env();

	if(env == NULL)
	{
		PARTYABNOR("env is NULL!");
		return;
	}

	//get curr stack sp
	char c;
	curr->stack_sp= &c;

	if (!pending_co->cIsShareStack)
	{
		env->pending_co = NULL;
		env->occupy_co = NULL;
	}
	else 
	{
		env->pending_co = pending_co;
		//get last occupy co on the same stack mem
		stCoRoutine_t* occupy_co = pending_co->stack_mem->occupy_co;
		//set pending co to occupy thest stack mem;
		pending_co->stack_mem->occupy_co = pending_co;

		env->occupy_co = occupy_co;
		if (occupy_co && occupy_co != pending_co)
		{
			save_stack_buffer(occupy_co);
		}
	}

	//swap context
	coctx_swap(&(curr->ctx), &(pending_co->ctx) );

	//stack buffer may be overwrite, so get again;
	stCoRoutineEnv_t* curr_env = co_get_curr_thread_env();
	stCoRoutine_t* update_occupy_co =  curr_env->occupy_co;
	stCoRoutine_t* update_pending_co = curr_env->pending_co;
	
	if (update_occupy_co && update_pending_co && update_occupy_co != update_pending_co)
	{
		//resume stack buffer
		if (update_pending_co->save_buffer && update_pending_co->save_size > 0)
		{
			memcpy(update_pending_co->stack_sp, update_pending_co->save_buffer, update_pending_co->save_size);
		}
	}
}

static stCoRoutineEnv_t* g_arrCoEnvPerThread[ TID_MAX ] = { 0 };
void co_init_curr_thread_env()
{
	pid_t pid = GetPid();

	g_arrCoEnvPerThread[ pid ] = (stCoRoutineEnv_t*)dave_ralloc( sizeof(stCoRoutineEnv_t) );
	stCoRoutineEnv_t *env = g_arrCoEnvPerThread[ pid ];

	env->iCallStackSize = 0;
	struct stCoRoutine_t *self = co_create_env( env, NULL, NULL, NULL );
	self->cIsMain = 1;

	env->pending_co = NULL;
	env->occupy_co = NULL;

	coctx_init( &self->ctx );

	env->pCallStack[ env->iCallStackSize ++ ] = self;
}

stCoRoutineEnv_t *co_get_curr_thread_env()
{
	return g_arrCoEnvPerThread[ GetPid() ];
}

stCoRoutine_t *GetCurrCo( stCoRoutineEnv_t *env )
{
	return env->pCallStack[ env->iCallStackSize - 1 ];
}

stCoRoutine_t *GetCurrThreadCo( )
{
	stCoRoutineEnv_t *env = co_get_curr_thread_env();
	if( !env ) return 0;
	return GetCurrCo(env);
}

stCoRoutine_t *co_self()
{
	return GetCurrThreadCo();
}

#endif

