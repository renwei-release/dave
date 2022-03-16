/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "dave_base.h"
#include "mem_lock.h"
#include "mem_exter.h"
#include "mem_mbuf.h"
#include "mem_log.h"
#include "mem_test.h"

extern ub base_thread_info(s8 *msg, ub msg_len);

static ub __memory_init_flag = 0x12345678;

static inline void
_base_memory_init(void)
{
	exter_mem_init();
}

static inline void
_base_memory_exit(void)
{
	exter_mem_exit();
}

static inline void *
____base_malloc____(ub len, dave_bool reset, s8 *file, ub line)
{
	void *ptr;
	
	if(len == 0)
	{
		return NULL;
	}

	ptr = __exter_malloc__(len, file, line);
	if(ptr != NULL)
	{
		return ptr;
	}

	return ptr;
}

static inline dave_bool
___base_free___(void *ptr, s8 *file, ub line)
{
	if(ptr == NULL)
	{
		MEMTRACE("Abnormal release resources, %s:%d", file, line);
		return dave_true;
	}

	if(__exter_free__(ptr, file, line) == dave_true)
	{
		return dave_true;
	}

	return dave_false;
}

static inline void
__base_mem_poweroff(s8 *file, ub line, ub len)
{
	s8 message[16384];
	ub message_index;

	base_mem_exit();

	message_index = 0;

	message_index += base_thread_info(&message[message_index], sizeof(message)-message_index);
	message_index += base_mem_info(&message[message_index], sizeof(message)-message_index, dave_false);

	dave_snprintf(&message[message_index], sizeof(message)-message_index,
		"\nLimited Memory, %s:%d malloc length:%ld",
		file, line, len);

	if(len == 0)
	{
		message_index += dave_snprintf(&message[message_index], sizeof(message) - message_index,
			"\n** Please note that <%s:%d> the memory allocation length is 0 **", file, line);
	}

	base_power_off((char *)message);
}

// =====================================================================

void *
__base_malloc__(ub len, dave_bool reset, u8 reset_data, s8 *file, ub line)
{
	void *ptr;

	ptr = ____base_malloc____(len, reset, file, line);

	if(ptr == NULL)
	{
		MEMABNOR("Limited resources,len:%ld %s:%d", len, file, line);

		__base_mem_poweroff(file, line, len);
	}
	else
	{
		if(reset == dave_true)
		{
			dave_memset(ptr, reset_data, len);
		}
	}

	return ptr;
}

void *
__base_calloc__(ub num, ub len, s8 *file, ub line)
{
	void *ptr;
	ub total_len = num * len;

	ptr = __base_malloc__(total_len, dave_true, 0x00, file, line);

	return ptr;
}

dave_bool
__base_free__(void *ptr, s8 *file, ub line)
{
	return ___base_free___(ptr, file, line);
}

MBUF *
__base_mmalloc__(ub length, s8 *file, ub line)
{
	return __mbuf_mmalloc__(length, file, line);
}

sb
__base_mheader__(MBUF *m, sb header_size_increment, s8 *file, ub line)
{
	return __mbuf_mheader__(m, header_size_increment, file, line);
}

ub
__base_mfree__(MBUF *m, s8 *file, ub line)
{
	return __mbuf_mfree__(m, file, line);
}

void
__base_mref__(MBUF *m, s8 *file, ub line)
{
	__mbuf_mref__(m, file, line);
}

MBUF *
__base_mchain__(MBUF *cur_point, MBUF *cat_point, s8 *file, ub line)
{
	return __mbuf_mchain__(cur_point, cat_point, file, line);
}

MBUF *
__base_mdechain__(MBUF *m, s8 *file, ub line)
{	
	return __mbuf_mdechain__(m, file, line);
}

MBUF *
__base_mclone__(MBUF *m, s8 *file, ub line)
{	
	return __mbuf_clone__(m, file, line);
}

void *
base_mptr(MBUF *data)
{
	if(data == NULL)
	{
		return NULL;
	}

	return data->payload;
}

ub
base_mlnumber(MBUF *data)
{
	return __mbuf_list_number__(data);
}

void
base_mem_init(void)
{
	dave_bool init_flag = dave_false;

	mem_lock();

	if(__memory_init_flag != 0xef1276ab)
	{
		__memory_init_flag = 0xef1276ab;
		init_flag = dave_true;
	}

	mem_unlock();

	if(init_flag == dave_true)
	{
		_base_memory_init();
	}
}

void
base_mem_exit(void)
{
	dave_bool exit_flag = dave_false;

	mem_lock();

	if(__memory_init_flag != 0x12345678)
	{
		__memory_init_flag = 0x12345678;
		exit_flag = dave_true;
	}

	mem_unlock();

	if(exit_flag == dave_true)
	{
		_base_memory_exit();
	}
}

ub
base_mem_info(s8 *info, ub info_len, dave_bool base_flag)
{
	ub info_index = 0;

	info_index += __exter_memory_info__(&info[info_index], info_len-info_index, base_flag);

	return info_index;
}

#endif

