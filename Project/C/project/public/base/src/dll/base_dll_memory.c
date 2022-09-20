/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "dave_3rdparty.h"
#include "base_dll_memory.h"

extern void *(*__malloc_hook)(size_t size);
extern void (*__free_hook)(void *ptr);
extern void *(*__block_mem_malloc__)(size_t size);
extern void (*__block_mem_free__)(void *ptr);

// =====================================================================

void
dave_dll_memory(void)
{
	printf("malloc:%lx __malloc_hook:%lx __block_mem_malloc__:%lx free:%lx __free_hook:%lx __block_mem_free__:%lx\n",
		(size_t)malloc, (size_t)__malloc_hook, (size_t)__block_mem_malloc__,
		(size_t)free, (size_t)__free_hook, (size_t)__block_mem_free__);
}

#endif

