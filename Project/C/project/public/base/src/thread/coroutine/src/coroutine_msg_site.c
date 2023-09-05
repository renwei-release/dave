/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_parameter.h"
#if defined(__DAVE_BASE__) && defined(ENABLE_THREAD_COROUTINE)
#include "thread_log.h"

typedef struct {
	void *pSite;
} MsgSite;

// =====================================================================

ub
coroutine_msg_site_malloc(void *pSite)
{
	MsgSite *pMsgSite = dave_malloc(sizeof(MsgSite));

	pMsgSite->pSite = pSite;

	return (ub)(pMsgSite);
}

ub
coroutine_msg_site_free(ub msg_site, void *pSite)
{
	MsgSite *pMsgSite = (MsgSite *)msg_site;

	if(pMsgSite->pSite != pSite)
	{
		THREADABNOR("invalid free:%lx/%lx", pMsgSite->pSite, pSite);
		return msg_site;
	}
	else
	{
		dave_free(pMsgSite);
		return 0;
	}
}

#endif

