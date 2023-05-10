/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_jemalloc.h"
#include "block_mem.h"

#if defined(PERFTOOLS_3RDPARTY)
#define TCMALLOC_ENABLE
#endif
#if defined(JEMALLOC_3RDPARTY)
#define JEMALLOC_ENABLE
#endif

#if defined(TCMALLOC_ENABLE)
#define BLOCK_MALLOC dave_perftools_malloc
#define BLOCK_FREE dave_perftools_free
#elif defined(JEMALLOC_ENABLE)
#define BLOCK_MALLOC dave_jemalloc
#define BLOCK_FREE dave_jefree
#define BLOCK_LEN dave_jelen
#else
#define BLOCK_MALLOC malloc
#define BLOCK_FREE free
#define BLOCK_LEN malloc_usable_size
#endif

#define OFFSET_LEN (16)
#define USERINDEX_LEN (32)
#define OVERFLOW_LEN (4)
#define OVERFLOW_CHAR (0xA5)
#define MEMADD_LEN (USERINDEX_LEN + OVERFLOW_LEN)

#define TOP_INFO_MAX 32

#define BLOCKMEMABNOR(a, ...) { DAVEABNORMAL("[BLOCK MEM Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define BLOCKMEMLOG(a, ...) { DAVELOG("[BLOCK MEM]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

block_mem_malloc_fun __block_mem_malloc__ = BLOCK_MALLOC;
block_mem_free_fun __block_mem_free__ = BLOCK_FREE;
block_mem_len_fun __block_mem_len__ = BLOCK_LEN;

static inline ub
_block_mem_base_info(char *block_name, s8 *info_ptr, ub info_len, BlockMem *pBlock)
{
	ub info_index = 0, block_index;
	ub core_number, total_number;

	core_number = total_number = 0;

	for(block_index=0; block_index<pBlock->block_number; block_index++)
	{
		core_number += pBlock[block_index].core_number;
		total_number += CORE_MEM_MAX;
	}

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		"%s MEMORY INFORMATION:%d\%(%d/%d)\n",
		block_name,
		(core_number * 100) / total_number, core_number, total_number);

	return info_index;
}

static inline ub
_block_mem_detail_info(char *block_name, s8 *info_ptr, ub info_len, BlockMem *pBlock)
{
	ub info_index = 0, block_index;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		"%s MEMORY INFORMATION:\n",
		block_name);

	for(block_index=0; block_index<pBlock->block_number; block_index++)
	{
		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" %02d/%02d free:(%03d/%03d) core:%02d\%(%d/%d)\n",
			pBlock[block_index].block_index, pBlock[block_index].block_number,
			pBlock[block_index].free_number, FREE_MEM_MAX,
			(pBlock[block_index].core_number * 100) / CORE_MEM_MAX, pBlock[block_index].core_number, CORE_MEM_MAX);
	}

	return info_index;
}

#ifdef MEM_RECORD_FILE_PTR
static inline ub
_block_mem_top_on_here(s8 *top_file[TOP_INFO_MAX], ub *top_line, s8 *file, ub line)
#else
static inline ub
_block_mem_top_on_here(s8 top_file[TOP_INFO_MAX][MEM_FILE_LEN], ub *top_line, s8 *file, ub line)
#endif
{
	ub top_index;

	for(top_index=0; top_index<TOP_INFO_MAX; top_index++)
	{
#ifdef MEM_RECORD_FILE_PTR
		if((top_file[top_index] == file) && (top_line[top_index] == line))
#else
		if((dave_strcmp(top_file[top_index], file) == dave_true) && (top_line[top_index] == line))
#endif
		{
			return top_index;
		}
	}

	return TOP_INFO_MAX;
}

#ifdef MEM_RECORD_FILE_PTR
static inline void
_block_mem_insert_new(s8 *top_file[TOP_INFO_MAX], ub *top_line, ub *top_number, ub *total_length, s8 *file, ub line, ub number, ub length)
#else
static inline void
_block_mem_insert_new(s8 top_file[TOP_INFO_MAX][MEM_FILE_LEN], ub *top_line, ub *top_number, ub *total_length, s8 *file, ub line, ub number, ub length)
#endif
{
	ub top_index, min_index, min_number;

	top_index = min_index = 0;
	min_number = top_number[top_index];

	top_index ++;

	while(top_index < TOP_INFO_MAX)
	{
		if(min_number > top_number[top_index])
		{
			min_index = top_index;
			min_number = top_number[top_index];
		}

		top_index ++;
	}

#ifdef MEM_RECORD_FILE_PTR
	top_file[min_index] = file;
#else
	dave_strcpy(top_file[min_index], file, MEM_FILE_LEN);
#endif
	top_line[min_index] = line;
	top_number[min_index] = number;
	total_length[min_index] = length;
}

#ifdef MEM_RECORD_FILE_PTR
static inline dave_bool
_block_mem_top_block_info(BlockMem *pBlock, s8 *top_file[TOP_INFO_MAX], ub *top_line, ub *top_number, ub *total_length)
#else
static inline dave_bool
_block_mem_top_block_info(BlockMem *pBlock, s8 top_file[TOP_INFO_MAX][MEM_FILE_LEN], ub *top_line, ub *top_number, ub *total_length)
#endif
{
	ub core_index;
	BlockMemCore *pCore;
	ub top_index;
	dave_bool has_top = dave_false;

	for(core_index=0; core_index<CORE_MEM_MAX; core_index++)
	{
		pCore = &(pBlock->core[core_index]);

		if(pCore->user_ptr != NULL)
		{
			top_index = _block_mem_top_on_here(top_file, top_line, pCore->m_file, pCore->m_line);

			if(top_index >= TOP_INFO_MAX)
			{
				_block_mem_insert_new(top_file, top_line, top_number, total_length, pCore->m_file, pCore->m_line, 1, pCore->len);
			}
			else
			{
				top_number[top_index] += 1;
				total_length[top_index] += pCore->len;
			}

			has_top = dave_true;
		}
	}

	return has_top;
}

#ifdef MEM_RECORD_FILE_PTR
static inline dave_bool
_block_mem_top_load(BlockMem *pBlock, s8 *top_file[TOP_INFO_MAX], ub *top_line, ub *top_number, ub *total_length)
#else
static inline dave_bool
_block_mem_top_load(BlockMem *pBlock, s8 top_file[TOP_INFO_MAX][MEM_FILE_LEN], ub *top_line, ub *top_number, ub *total_length)
#endif
{
	ub block_index;
	dave_bool has_top = dave_false;

	for(block_index=0; block_index<pBlock->block_number; block_index++)
	{
		if(_block_mem_top_block_info(&pBlock[block_index], top_file, top_line, top_number, total_length) == dave_true)
		{
			has_top = dave_true;
		}
	}

	return has_top;
}

#ifdef MEM_RECORD_FILE_PTR
static inline void
_block_mem_top_sort(s8 *top_file[TOP_INFO_MAX], ub *top_line, ub *top_number, ub *total_length)
#else
static inline void
_block_mem_top_sort(s8 top_file[TOP_INFO_MAX][MEM_FILE_LEN], ub *top_line, ub *top_number, ub *total_length)
#endif
{
	ub top_1_index, top_2_index;
	#ifdef MEM_RECORD_FILE_PTR
	s8 *file;
	#else
	s8 file[MEM_FILE_LEN];
	#endif
	ub line, number, length;

	for(top_1_index=0; top_1_index<TOP_INFO_MAX; top_1_index++)
	{
		if(top_file[top_1_index] == NULL)
			break;
	
		for(top_2_index=top_1_index+1; top_2_index<TOP_INFO_MAX; top_2_index++)
		{
			if(top_file[top_2_index] == NULL)
				break;

			if(top_number[top_2_index] > top_number[top_1_index])
			{
				#ifdef MEM_RECORD_FILE_PTR
				file = top_file[top_1_index];
				#else
				dave_strcpy(file, top_file[top_1_index], MEM_FILE_LEN);
				#endif
				line = top_line[top_1_index];
				number = top_number[top_1_index];
				length = total_length[top_1_index];

				#ifdef MEM_RECORD_FILE_PTR
				top_file[top_1_index] = top_file[top_2_index];
				#else
				dave_strcpy(top_file[top_1_index], top_file[top_2_index], MEM_FILE_LEN);
				#endif
				top_line[top_1_index] = top_line[top_2_index];
				top_number[top_1_index] = top_number[top_2_index];
				total_length[top_1_index] = total_length[top_2_index];

				#ifdef MEM_RECORD_FILE_PTR
				top_file[top_2_index] = file;
				#else
				dave_strcpy(top_file[top_2_index], file, MEM_FILE_LEN);
				#endif
				top_line[top_2_index] = line;
				top_number[top_2_index] = number;
				total_length[top_2_index] = length;
			}
		}
	}
}

static inline ub
_block_mem_top_info(char *block_name, s8 *info_ptr, ub info_len, BlockMem *pBlock, ub warning_number_exceeded)
{
	ub info_index, top_index;
	#ifdef MEM_RECORD_FILE_PTR
	s8 *top_file[TOP_INFO_MAX];
	#else
	s8 top_file[TOP_INFO_MAX][MEM_FILE_LEN];
	#endif
	ub top_line[TOP_INFO_MAX];
	ub top_number[TOP_INFO_MAX];
	ub total_length[TOP_INFO_MAX];
	dave_bool has_top = dave_false;
	dave_bool has_number = dave_false;

	dave_memset(top_file, 0x00, sizeof(top_file));
	dave_memset(top_line, 0x00, sizeof(top_line));
	dave_memset(top_number, 0x00, sizeof(top_number));
	dave_memset(total_length, 0x00, sizeof(total_length));

	has_top = _block_mem_top_load(pBlock, top_file, top_line, top_number, total_length);

	info_index = 0;

	if(has_top == dave_true)
	{
		_block_mem_top_sort(top_file, top_line, top_number, total_length);

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			"%s MEMORY TOP:\n", block_name);

		for(top_index=0; top_index<TOP_INFO_MAX; top_index++)
		{
			if(top_number[top_index] > warning_number_exceeded)
			{
				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
					" %s:%d total malloc -> (number:%lu length:%lu)\n",
					top_file[top_index], top_line[top_index], top_number[top_index], total_length[top_index]);

				has_number = dave_true;
			}
		}
	}

	if((has_top == dave_false) || (has_number == dave_false))
		return 0;

	return info_index;
}

static inline ub
_block_mem_user_ptr_to_block_index(void *user_ptr)
{
	return ((ub *)((u8 *)user_ptr - USERINDEX_LEN))[0];
}

static inline ub
_block_mem_user_ptr_to_core_index(void *user_ptr)
{
	return ((ub *)((u8 *)user_ptr - USERINDEX_LEN))[1];
}

static inline void
_block_mem_core_reset(BlockMemCore *pCore)
{
	pCore->user_ptr = NULL;
	pCore->len = 0;

#ifdef MEM_RECORD_FILE_PTR
	pCore->m_file = NULL;
	pCore->m_line = 0;
	pCore->f_file = NULL;
	pCore->f_line = 0;
#else
	pCore->m_file[0] = '\0';
	pCore->m_line = 0;
	pCore->f_file[0] = '\0';
	pCore->f_line = 0;
#endif
}

static inline void
_block_mem_reset(ub block_index, ub block_number, BlockMem *pBlock)
{
	ub free_index, core_index;

	pBlock->block_index = block_index;
	pBlock->block_number = block_number;

	pBlock->core_number = 0;
	pBlock->core_search_index = 0;
	pBlock->free_number = 0;
	for(free_index=0; free_index<FREE_MEM_MAX; free_index++)
	{
		pBlock->free_index[free_index] = CORE_MEM_MAX;
	}

	for(core_index=0; core_index<CORE_MEM_MAX; core_index++)
	{
		_block_mem_core_reset(&(pBlock->core[core_index]));
	}
}

static inline void
_block_mem_add_overflow(void *ptr, ub len)
{
	if(ptr == NULL) return;

	(((u8 *)ptr)[len + 0]) = OVERFLOW_CHAR;
	(((u8 *)ptr)[len + 1]) = OVERFLOW_CHAR;
	(((u8 *)ptr)[len + 2]) = OVERFLOW_CHAR;
	(((u8 *)ptr)[len + 3]) = OVERFLOW_CHAR;
}

static inline dave_bool
_block_mem_check_overflow(void *ptr, ub len)
{
	if(((u8 *)ptr)[len + 0] != OVERFLOW_CHAR)
	{
		return dave_false;
	}
	if(((u8 *)ptr)[len + 1] != OVERFLOW_CHAR)
	{
		return dave_false;
	}
	if(((u8 *)ptr)[len + 2] != OVERFLOW_CHAR)
	{
		return dave_false;
	}
	if(((u8 *)ptr)[len + 3] != OVERFLOW_CHAR)
	{
		return dave_false;
	}

	return dave_true;
}

static inline ub
_block_mem_free_get(ub core_index, BlockMem *pBlock)
{
	ub free_index;

	if(pBlock->free_number < 0)
	{
		BLOCKMEMABNOR("invalid free_number:%d", pBlock->free_number);
		return core_index;
	}
	if(pBlock->free_number == 0)
	{
		return core_index;
	}

	for(free_index=0; free_index<FREE_MEM_MAX; free_index++)
	{
		if(pBlock->free_index[free_index] < CORE_MEM_MAX)
		{
			core_index = pBlock->free_index[free_index];
			pBlock->free_index[free_index] = CORE_MEM_MAX;
			pBlock->free_number --;
			return core_index;
		}
	}

	BLOCKMEMABNOR("free_number:%d not zero! but can't find index, why?", pBlock->free_number);
	return core_index;
}

static inline void
_block_mem_free_set(ub core_index, BlockMem *pBlock)
{
	ub free_index;

	if(pBlock->free_number > FREE_MEM_MAX)
	{
		BLOCKMEMABNOR("invalid free_number:%d", pBlock->free_number);
		return;
	}
	if(pBlock->free_number == FREE_MEM_MAX)
	{
		return;
	}

	for(free_index=0; free_index<FREE_MEM_MAX; free_index++)
	{
		if(pBlock->free_index[free_index] >= CORE_MEM_MAX)
		{
			pBlock->free_index[free_index] = core_index;
			pBlock->free_number ++;
			return;
		}
	}

	BLOCKMEMABNOR("free_number:%d can be set! but can't find index, why?", pBlock->free_number);
}

static inline void *
_block_mem_malloc(ub *core_index, void *ptr, void *user_ptr, BlockMem *pBlock, ub len, s8 *file, ub line)
{
	ub safe_counter;
	BlockMemCore *pCore;

	if(pBlock->core_number >= CORE_MEM_MAX)
	{
		__block_mem_free__(ptr);
		return NULL;
	}

	for(safe_counter=0; safe_counter<CORE_MEM_MAX; safe_counter++)
	{
		if(pBlock->core_search_index >= CORE_MEM_MAX)
		{
			pBlock->core_search_index = 0;
		}

		pCore = &(pBlock->core[pBlock->core_search_index]);

		if(pCore->user_ptr == NULL)
		{
			pCore->ptr = ptr;
			pCore->user_ptr = user_ptr;
			pCore->len = len;

			#ifdef MEM_RECORD_FILE_PTR
			pCore->m_file = file;
			#else
			dave_strcpy(pCore->m_file, file, MEM_FILE_LEN);
			#endif
			pCore->m_line = line;

			pBlock->core_number ++;

			*core_index = pBlock->core_search_index ++;

			return user_ptr;
		}

		pBlock->core_search_index = _block_mem_free_get(++ pBlock->core_search_index, pBlock);
	}

	__block_mem_free__(ptr);
	return NULL;
}

static inline void *
_block_mem_free(ub core_index, BlockMem *pBlock, s8 *file, ub line)
{
	BlockMemCore *pCore = &(pBlock->core[core_index]);
	void *ptr = pCore->ptr;

	pCore->user_ptr = NULL;
	pCore->len = 0;

	#ifdef MEM_RECORD_FILE_PTR
	pCore->f_file = file;
	#else
	dave_strcpy(pCore->f_file, file, MEM_FILE_LEN);
	#endif
	pCore->f_line = line;

	if(pBlock->core_number <= 0)
	{
		BLOCKMEMABNOR("Arithmetic error! %d", pBlock->core_number);
	}
	else
	{
		_block_mem_free_set(core_index, pBlock);

		pBlock->core_number --;
	}

	return ptr;
}

static inline dave_bool
_block_mem_memory(ub core_index, void *user_ptr, BlockMem *pBlock)
{
	if(core_index >= CORE_MEM_MAX)
	{
		return dave_false;
	}

	if(pBlock->core[core_index].user_ptr != user_ptr)
	{
		return dave_false;
	}

	return dave_true;
}

static inline void *
_block_mem_safe_malloc(BlockMem *pBlock, ub len, s8 *file, ub line)
{
	void *ptr, *offset_ptr, *user_ptr;
	ub malloc_len = OFFSET_LEN + len + MEMADD_LEN;

	ptr = __block_mem_malloc__(malloc_len);
	if(ptr == NULL)
	{
		return NULL;
	}

	offset_ptr = (void *)(((ub)ptr + OFFSET_LEN) & 0xfffffffffffffff0);

	((ub *)offset_ptr)[0] = pBlock->block_index;

	pthread_spin_lock((pthread_spinlock_t *)(pBlock->opt_pv.spin_lock));
	{ user_ptr = _block_mem_malloc(&(((ub *)offset_ptr)[1]), ptr, &(((u8 *)offset_ptr)[USERINDEX_LEN]), pBlock, len, file, line); }
	pthread_spin_unlock((pthread_spinlock_t *)(pBlock->opt_pv.spin_lock));

	_block_mem_add_overflow(user_ptr, len);

	return user_ptr;
}

static inline dave_bool
_block_mem_safe_free(BlockMem *pBlock, void *user_ptr, s8 *file, ub line)
{
	ub core_index;
	BlockMemCore *pCore;
	void *ptr = NULL;

	core_index = _block_mem_user_ptr_to_core_index(user_ptr);
	if(core_index >= CORE_MEM_MAX)
	{
		BLOCKMEMABNOR("find invalid core_index:%d user_ptr:%lx <%s:%d>",
			core_index, user_ptr, file, line);
		return dave_false;
	}

	pCore = &(pBlock->core[core_index]);
	if(pCore->user_ptr != user_ptr)
	{
		BLOCKMEMABNOR("the ptr:%lx/%lx free failed! len:%d (c-%s:%d/m-%s:%d/f-%s:%d)",
			user_ptr, pCore->user_ptr,
			pCore->len,
			file, line,
			pCore->m_file, pCore->m_line,
			pCore->f_file, pCore->f_line);
		return dave_false;
	}
	if(_block_mem_check_overflow(user_ptr, pCore->len) == dave_false)
	{
		BLOCKMEMABNOR("%d Overflow<%x>:(c-%s:%d/m-%s:%d/f-%s:%d)",
			pCore->len,
			((u8 *)(pCore->user_ptr))[pCore->len],
			file, line,
			pCore->m_file, pCore->m_line,
			pCore->f_file, pCore->f_line);
	}

	pthread_spin_lock((pthread_spinlock_t *)(pBlock->opt_pv.spin_lock));
	{ ptr = _block_mem_free(core_index, pBlock, file, line); }
	pthread_spin_unlock((pthread_spinlock_t *)(pBlock->opt_pv.spin_lock));

	if(ptr == NULL)
	{
		BLOCKMEMABNOR("the ptr:%lx free failed! <%s:%d>",
			user_ptr, file, line);
		return dave_false;
	}

	__block_mem_free__(ptr);

	return dave_true;
}

static inline ub
_block_mem_safe_len(BlockMem *pBlock, void *user_ptr, s8 *file, ub line)
{
	ub core_index;
	BlockMemCore *pCore;

	core_index = _block_mem_user_ptr_to_core_index(user_ptr);
	if(core_index >= CORE_MEM_MAX)
	{
		BLOCKMEMABNOR("find invalid core_index:%d user_ptr:%lx <%s:%d>",
			core_index, user_ptr, file, line);
		return 0;
	}

	pCore = &(pBlock->core[core_index]);
	if(pCore->user_ptr != user_ptr)
	{
		BLOCKMEMABNOR("the ptr:%lx/%lx free failed! len:%d (c-%s:%d/m-%s:%d/f-%s:%d)",
			user_ptr, pCore->user_ptr,
			pCore->len,
			file, line,
			pCore->m_file, pCore->m_line,
			pCore->f_file, pCore->f_line);
		return 0;
	}

	return pCore->len;
}

static inline dave_bool
_block_mem_safe_memory(BlockMem *pBlock, void *user_ptr)
{
	return _block_mem_memory(_block_mem_user_ptr_to_core_index(user_ptr), user_ptr, pBlock);
}

// =====================================================================

void
block_mem_reset(BlockMem *pBlock, ub block_number)
{
	ub block_index;

	dave_memset(pBlock, 0x00, block_number * sizeof(BlockMem));

	for(block_index=0; block_index<block_number; block_index++)
	{
		t_lock_reset(&(pBlock[block_index].opt_pv));

		_block_mem_reset(block_index, block_number, &pBlock[block_index]);
	}
}

void *
block_malloc(BlockMem *pBlock, ub len, s8 *file, ub line)
{
	static ub block_index = 0;
	ub safe_counter;
	void *user_ptr;

	safe_counter = 0;
	user_ptr = NULL;

	while((user_ptr == NULL) && ((safe_counter ++) < pBlock->block_number))
	{
		user_ptr = _block_mem_safe_malloc(&pBlock[(block_index ++) % pBlock->block_number], len, file, line);
	}

	return user_ptr;
}

dave_bool
block_free(BlockMem *pBlock, void *user_ptr, s8 *file, ub line)
{
	ub block_index = _block_mem_user_ptr_to_block_index(user_ptr);

	if(block_index >= pBlock->block_number)
	{
		BLOCKMEMABNOR("invalid block_index:%d block_number:%d user_ptr:%x <%s:%d>",
			block_index, pBlock->block_number, user_ptr, file, line);
		return dave_false;
	}
	if(block_index != pBlock[block_index].block_index)
	{
		BLOCKMEMABNOR("invalid block_index:%d/%d/%d user_ptr:%x <%s:%d>",
			block_index, pBlock[block_index].block_index, pBlock->block_number,
			user_ptr,
			file, line);
		return dave_false;
	}

	return _block_mem_safe_free(&(pBlock[block_index]), user_ptr, file, line);
}

ub
block_len(BlockMem *pBlock, void *user_ptr, s8 *file, ub line)
{
	ub block_index = _block_mem_user_ptr_to_block_index(user_ptr);

	if(block_index >= pBlock->block_number)
	{
		BLOCKMEMABNOR("invalid block_index:%d block_number:%d user_ptr:%x <%s:%d>",
			block_index, pBlock->block_number, user_ptr, file, line);
		return dave_false;
	}
	if(block_index != pBlock[block_index].block_index)
	{
		BLOCKMEMABNOR("invalid block_index:%d/%d/%d user_ptr:%x <%s:%d>",
			block_index, pBlock[block_index].block_index, pBlock->block_number,
			user_ptr,
			file, line);
		return dave_false;
	}

	return _block_mem_safe_len(&(pBlock[block_index]), user_ptr, file, line);
}

dave_bool
block_memory(BlockMem *pBlock, void *user_ptr, s8 *file, ub line)
{
	ub block_index = _block_mem_user_ptr_to_block_index(user_ptr);

	if(block_index >= pBlock->block_number)
	{
		return dave_false;
	}
	if(block_index != pBlock[block_index].block_index)
	{
		return dave_false;
	}

	return _block_mem_safe_memory(&(pBlock[block_index]), user_ptr);
}

ub
block_info(char *block_name, BlockMem *pBlock, s8 *info_ptr, ub info_len, dave_bool base_flag, dave_bool detail_flag, ub warning_number_exceeded)
{
	ub info_index;

	if(base_flag == dave_true)
	{
		info_index = _block_mem_base_info(block_name, info_ptr, info_len, pBlock);
	}
	else
	{
		info_index = _block_mem_top_info(block_name, info_ptr, info_len, pBlock, warning_number_exceeded);
		if((info_index > 0) && (detail_flag == dave_true))
		{
			BLOCKMEMLOG("\nPlease note that the following memory has not been released(%s):\n%s", block_name, info_ptr);

			info_index += _block_mem_detail_info(block_name, &info_ptr[info_index], info_len-info_index, pBlock);
		}
	}

	return info_index;
}

void
block_info_write(char *block_name, BlockMem *pBlock)
{
	s8 info_ptr[16384];
	ub info_len;
	DateStruct date;
	s8 file_name[64];

	info_len = block_info(block_name, pBlock, info_ptr, sizeof(info_ptr), dave_false, dave_true, 64);

	if(info_len > 0)
	{
		t_time_get_date(&date);
		dave_snprintf(file_name, sizeof(file_name),
			"%s-%04d-%02d-%02d_%02d-%02d-%02d",
			block_name,
			date.year, date.month, date.day,
			date.hour, date.minute, date.second);
	
		dave_os_file_write(CREAT_WRITE_FLAG, file_name, 0xffffffff, info_len, (u8 *)info_ptr);
	}
}

#endif

