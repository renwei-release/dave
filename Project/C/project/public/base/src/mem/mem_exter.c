/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_tools.h"
#include "base_tools.h"

#ifdef LEVEL_PRODUCT_alpha
 #define EXTER_ENABLE_BLOCK
#endif

#ifdef EXTER_ENABLE_BLOCK
#define EXTER_MEM_MAX 16
#define EXTER_MEM_NAME "EXTER"

static BlockMem _exter_mem[EXTER_MEM_MAX];

static inline void
_exter_mem_init(void)
{
	static volatile sb __safe_pre_flag__ = 0;

	SAFEPre(__safe_pre_flag__, { block_mem_reset(_exter_mem, EXTER_MEM_MAX); });
}
#endif

// =====================================================================

void
exter_mem_init(void)
{
#ifdef EXTER_ENABLE_BLOCK
	_exter_mem_init();
#endif
}

void
exter_mem_exit(void)
{
#ifdef EXTER_ENABLE_BLOCK
	block_info_write(EXTER_MEM_NAME, _exter_mem);
#endif
}

void *
__exter_malloc__(ub len, s8 *file, ub line)
{
#ifdef EXTER_ENABLE_BLOCK
	_exter_mem_init();
	return block_malloc(_exter_mem, len, file, line);
#else
	return __block_mem_malloc__((size_t)len);
#endif
}

dave_bool
__exter_free__(void *ptr, s8 *file, ub line)
{
#ifdef EXTER_ENABLE_BLOCK
	_exter_mem_init();
	return block_free(_exter_mem, ptr, file, line);
#else
	__block_mem_free__(ptr);
	return dave_true;
#endif
}

ub
__exter_len__(void *ptr, s8 *file, ub line)
{
#ifdef EXTER_ENABLE_BLOCK
	_exter_mem_init();
	return block_len(_exter_mem, ptr, file, line);
#else
	return __block_mem_len__(ptr);
#endif
}

dave_bool
__exter_memory__(void *ptr, s8 *file, ub line)
{
#ifdef EXTER_ENABLE_BLOCK
	_exter_mem_init();
	return block_memory(_exter_mem, ptr, file, line);
#else
	return dave_true;
#endif
}

ub
__exter_memory_info__(s8 *info_ptr, ub info_len, dave_bool base_flag)
{
#ifdef EXTER_ENABLE_BLOCK
	return block_info(EXTER_MEM_NAME, _exter_mem, info_ptr, info_len, base_flag, dave_false, 0);
#else
	return 0;
#endif
}

#endif

