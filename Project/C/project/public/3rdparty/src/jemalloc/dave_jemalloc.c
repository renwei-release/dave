/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(JEMALLOC_3RDPARTY)
#include "jemalloc/internal/jemalloc_preamble.h"
#include "jemalloc/internal/jemalloc_internal_includes.h"

#include "jemalloc/internal/conflict_rename_assert.h"
#include "jemalloc/internal/decay.h"
#include "jemalloc/internal/ehooks.h"
#include "jemalloc/internal/extent_dss.h"
#include "jemalloc/internal/extent_mmap.h"
#include "jemalloc/internal/san.h"
#include "jemalloc/internal/mutex.h"
#include "jemalloc/internal/rtree.h"
#include "jemalloc/internal/safety_check.h"
#include "jemalloc/internal/util.h"


// =====================================================================

void *
dave_jemalloc(size_t size)
{
	return je_malloc(size);
}

void
dave_jefree(void *ptr)
{
	je_free(ptr);
}

size_t
dave_jelen(void *ptr)
{
	return je_malloc_usable_size(ptr);
}

#endif

