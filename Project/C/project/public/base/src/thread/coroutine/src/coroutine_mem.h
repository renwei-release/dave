/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __COROUTINE_MEM_H__
#define __COROUTINE_MEM_H__

void coroutine_mem_init(void);

void coroutine_mem_exit(void);

void * coroutine_malloc(ub length);

void coroutine_free(void *ptr);

#endif

