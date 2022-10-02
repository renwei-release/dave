/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __COROUTINE_CORE_H__
#define __COROUTINE_CORE_H__

typedef void * (* coroutine_core_fun)(void *param);

void coroutine_core_init(void);

void coroutine_core_exit(void);

void * coroutine_create(coroutine_core_fun fun_ptr, void *fun_param);

void coroutine_resume(void *co);

void coroutine_yield(void *co);

void coroutine_release(void *co);

#endif

