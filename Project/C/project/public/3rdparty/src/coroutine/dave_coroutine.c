/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(COROUTINE_3RDPARTY)
#include "dave_base.h"
#include "dave_coroutine.h"

int co_create( void **co,const void *attr,void *routine,void *arg );
void co_resume( void *co );
void co_yield( void *co );
void co_release( void *co );

// =====================================================================

void *
dave_co_create(coroutine_fun fun, void *param)
{
	void *co = NULL;

	co_create(&co, NULL, (void *)fun, param);

	return (void *)co;
}

void
dave_co_resume(void *co)
{
	if(co != NULL)
	{
		co_resume(co);
	}
}

void
dave_co_yield(void *co)
{
	if(co != NULL)
	{
		co_yield(co);
	}
}

void
dave_co_release(void *co)
{
	if(co != NULL)
	{
		co_release(co);
	}
}

#endif

