/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.12.10.
 * ================================================================================
 */

#ifndef __BLOCK_MEM_H__
#define __BLOCK_MEM_H__
#include "dave_third_party.h"

#if defined(PERFTOOLS_3RDPARTY)
#define TCMALLOC_ENABLE
#endif
#if defined(JEMALLOC_3RDPARTY)
#define JEMALLOC_ENABLE
#endif

#if defined(TCMALLOC_ENABLE)
#define BLOCK_MALLOC(len) dave_perftools_malloc(len)
#define BLOCK_FREE(ptr) dave_perftools_free(ptr)
#define BLOCK_SIZE(ptr) dave_perftools_size(ptr)
#elif defined(JEMALLOC_ENABLE)
#define BLOCK_MALLOC(len) dave_jemalloc(len)
#define BLOCK_FREE(ptr) dave_jefree(ptr)
#define BLOCK_SIZE(ptr) dave_jelen(ptr)
#else
#define BLOCK_MALLOC(len) dave_os_malloc(len)
#define BLOCK_FREE(ptr) dave_os_free(ptr)
#define BLOCK_SIZE(ptr) dave_os_size(ptr)
#endif

#define CORE_MEM_MAX 204800
#define FREE_MEM_MAX 32

typedef struct {
	void *user_ptr;
	ub len;

	s8 *m_file;
	ub m_line;
	s8 *f_file;
	ub f_line;
} BlockMemCore;

typedef struct {
	TLock opt_pv;

	ub block_index;
	ub block_number;

	sb core_number;
	ub core_search_index;
	sb free_number;
	ub free_index[FREE_MEM_MAX];	

	BlockMemCore core[CORE_MEM_MAX];
} BlockMem;

void block_mem_reset(BlockMem *pBlock, ub block_number);

void * block_malloc(BlockMem *pBlock, ub len, s8 *file, ub line);

dave_bool block_free(BlockMem *pBlock, void *user_ptr, s8 *file, ub line);

dave_bool block_memory(BlockMem *pBlock, void *user_ptr, s8 *file, ub line);

ub block_info(char *block_name, BlockMem *pBlock, s8 *info_ptr, ub info_len, dave_bool base_flag, dave_bool detail_flag);

void block_info_write(char *block_name, BlockMem *pBlock);

#endif

