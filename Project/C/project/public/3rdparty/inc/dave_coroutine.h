/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_COROUTINE_H__
#define __DAVE_COROUTINE_H__

typedef void * (* coroutine_fun)(void *param);

void * dave_co_create(coroutine_fun fun, void *param);

void dave_co_resume(void *co);

void dave_co_yield(void *co);

void dave_co_release(void *co);

#endif

