/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "third_party_macro.h"
#if defined(JEMALLOC_3RDPARTY)
#include "dave_os.h"
#include "party_log.h"
#include "jemalloc.h"

// =====================================================================

void *
dave_jemalloc(ub length)
{
	return (void *)malloc((size_t)length);
}

void
dave_jefree(void *ptr)
{
	free(ptr);
}

ub
dave_jelen(void *ptr)
{
	return (ub)malloc_usable_size(ptr);
}

#endif

