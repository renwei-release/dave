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

void coroutine_core_creat(void);

void coroutine_core_die(void);

void coroutine_set_stack_size(ub size);

ub coroutine_get_stack_size(void);

void * coroutine_create(coroutine_core_fun fun_core, void *fun_param, MSGBODY *msg);

dave_bool coroutine_resume(void *co);

dave_bool coroutine_yield(void *co);

void coroutine_release(void *co);

dave_bool coroutine_be_in_use(void *co);

#endif

