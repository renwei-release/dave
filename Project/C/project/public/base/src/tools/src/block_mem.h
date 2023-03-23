/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BLOCK_MEM_H__
#define __BLOCK_MEM_H__
#include "dave_3rdparty.h"
#include "dave_os.h"

#define CORE_MEM_MAX 204800
#define FREE_MEM_MAX 32

typedef struct {
	void *ptr;
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

typedef void * (* block_mem_malloc_fun)(size_t length);
typedef void (* block_mem_free_fun)(void *ptr);
typedef ub (* block_mem_len_fun)(void *ptr);

extern block_mem_malloc_fun __block_mem_malloc__;
extern block_mem_free_fun __block_mem_free__;
extern block_mem_len_fun __block_mem_len__;

void block_mem_reset(BlockMem *pBlock, ub block_number);

void * block_malloc(BlockMem *pBlock, ub len, s8 *file, ub line);

dave_bool block_free(BlockMem *pBlock, void *user_ptr, s8 *file, ub line);

ub block_len(BlockMem *pBlock, void *user_ptr, s8 *file, ub line);

dave_bool block_memory(BlockMem *pBlock, void *user_ptr, s8 *file, ub line);

ub block_info(char *block_name, BlockMem *pBlock, s8 *info_ptr, ub info_len, dave_bool base_flag, dave_bool detail_flag, ub warning_number_exceeded);

void block_info_write(char *block_name, BlockMem *pBlock);

#endif

