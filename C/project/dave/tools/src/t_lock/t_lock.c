/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.07.25.
 * ================================================================================
 */

#include <pthread.h>
#include "dave_tools.h"
#include "dave_os.h"
#include "tools_log.h"

static TLock _t_booting_lock;

#ifdef __TOOLS_ALPHA_VERSION__

static inline dave_bool
_t_lock_check(TLock *pLock, s8 *fun, ub line)
{
	dave_bool ret;

	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSABNOR("Found an invalid lock[%s:%d, pLock:%x]!", fun, line, pLock);
		return dave_false;
	}

	ret = dave_true;

	t_lock_spin(pLock);
	if(pLock->thread_id == -1)
	{
		pLock->thread_id = pthread_self();
		pLock->file = fun;
		pLock->line = line;
	}
	else if(pLock->thread_id == pthread_self())
	{
		TOOLSABNOR("%s:%d->%s:%d a nested lock occurred or lock was not released correctly!", pLock->file, pLock->line, fun, line);
		ret = dave_false;
	}
	t_unlock_spin(pLock);

	return ret;
}

static inline dave_bool
_t_unlock_check(TLock *pLock, s8 *fun, ub line)
{
	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSABNOR("Found an invalid lock! [%s:%d magic:%lx/%lx]",
			fun, line, pLock->magic_data_1, pLock->magic_data_2);
		return dave_false;
	}

	t_lock_spin(pLock);
	if(pLock->thread_id == pthread_self())
	{
		pLock->thread_id = -1;
		pLock->file = NULL;
		pLock->line = 0;
	}
	t_unlock_spin(pLock);

	return dave_true;
}

#endif

// =====================================================================

void
t_lock_booting(void)
{
	t_lock_reset(&_t_booting_lock);
}

void
t_lock_reset(TLock *pLock)
{
	dave_memset(pLock, 0x00, sizeof(TLock));

#ifdef __TOOLS_ALPHA_VERSION__
	pLock->magic_data_1 = dave_rand();
#endif

	dave_os_spin_lock_init((void *)(pLock->spin_lock));
	dave_os_rw_lock_init((void *)(pLock->rw_lock));
	dave_os_mutex_init(&(pLock->m_mutex_t));

#ifdef __TOOLS_ALPHA_VERSION__
	pLock->thread_id = -1;
	pLock->file = NULL;
	pLock->line = 0;
	pLock->magic_data_2 = pLock->magic_data_1;
#endif
}

void
t_lock_destroy(TLock *pLock)
{
#ifdef __TOOLS_ALPHA_VERSION__
	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSABNOR("Found an invalid lock!");
		return;
	}
#endif

	dave_os_spin_lock_destroy((void *)(pLock->spin_lock));
	dave_os_rw_lock_destroy((void *)(pLock->rw_lock));
	dave_os_mutex_destroy(&(pLock->m_mutex_t));
}

void
__t_lock_spin__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSABNOR("Found an invalid lock[%s:%d, pLock:%x]!", fun, line, pLock);
		return;
	}
#endif

	dave_os_spin_lock(pLock->spin_lock);
}

void
__t_unlock_spin__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(pLock->magic_data_1 != pLock->magic_data_2)
	{
		TOOLSABNOR("Found an invalid lock[%s:%d, pLock:%x]!", fun, line, pLock);
		return;
	}
#endif

	dave_os_spin_unlock(pLock->spin_lock);
}

dave_bool
__t_rlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(_t_lock_check(pLock, fun, line) == dave_true)
#endif
		return dave_os_rw_rlock(pLock->rw_lock);
#ifdef __TOOLS_ALPHA_VERSION__
	else
		return dave_false;
#endif
}

dave_bool
__t_wlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(_t_lock_check(pLock, fun, line) == dave_true)
#endif
		return dave_os_rw_wlock(pLock->rw_lock);
#ifdef __TOOLS_ALPHA_VERSION__
	else
		return dave_false;
#endif
}

dave_bool
__t_trlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	dave_bool ret;

	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(_t_lock_check(pLock, fun, line) == dave_true)
	{
#endif
		ret = dave_os_rw_tryrlock(pLock->rw_lock);
#ifdef __TOOLS_ALPHA_VERSION__
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

dave_bool
__t_twlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	dave_bool ret;

	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(_t_lock_check(pLock, fun, line) == dave_true)
	{
#endif
		ret = dave_os_rw_trywlock(pLock->rw_lock);
#ifdef __TOOLS_ALPHA_VERSION__
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

dave_bool
__t_unlock_rw__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(_t_unlock_check(pLock, fun, line) == dave_true)
#endif
		return dave_os_rw_unlock(pLock->rw_lock);
#ifdef __TOOLS_ALPHA_VERSION__
	else
		return dave_false;
#endif
}

void
__t_lock_mutex__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(_t_lock_check(pLock, fun, line) == dave_true)
#endif
	dave_os_mutex_lock(&(pLock->m_mutex_t));
}

dave_bool
__t_trylock_mutex__(TLock *pLock, s8 *fun, ub line)
{
	dave_bool ret;

	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(_t_lock_check(pLock, fun, line) == dave_true)
	{
#endif
		ret = dave_os_mutex_trylock(&(pLock->m_mutex_t));
#ifdef __TOOLS_ALPHA_VERSION__
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

void
__t_unlock_mutex__(TLock *pLock, s8 *fun, ub line)
{
	if(pLock == NULL)
	{
		pLock = &_t_booting_lock;
	}

#ifdef __TOOLS_ALPHA_VERSION__
	if(_t_unlock_check(pLock, fun, line) == dave_true)
#endif
	dave_os_mutex_unlock(&(pLock->m_mutex_t));
}

