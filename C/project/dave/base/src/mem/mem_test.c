/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "mem_test.h"
#include "mem_log.h"

static void
_mem_test_memory_len_(ub len)
{
#if defined(JEMALLOC_3RDPARTY)
	void *ptr;

	ptr = dave_jemalloc(len);

	MEMLOG("ptr len:%d/%d", len, dave_jelen(ptr));

	dave_jefree(ptr);
#endif
}

static void
_mem_test_memory_len(void)
{
	ub len;

	for(len=1; len<112; len++)
	{
		_mem_test_memory_len_(len);
	}

	for(len=10000; len<10032; len++)
	{
		_mem_test_memory_len_(len);
	}
}

static void
_mem_test_memory_mbuf(void)
{
	MBUF *mbuf_ptr = NULL;
	MBUF *mbuf_clone;

	mbuf_ptr = dave_mchain(mbuf_ptr, dave_mmalloc(1024));
	mbuf_ptr = dave_mchain(mbuf_ptr, dave_mmalloc(1024));
	mbuf_ptr = dave_mchain(mbuf_ptr, dave_mmalloc(1024));

	mbuf_clone = dave_mclone(mbuf_ptr);

	dave_mfree(mbuf_ptr);
	dave_mfree(mbuf_clone);
}

static void
_mem_test_memory_overflow(void)
{
	s8 *string_buf;

	string_buf = dave_malloc(9);

	dave_memcpy(string_buf, "0123456789", 10);

	dave_free(string_buf);
}

static void
_mem_test_memory_double_free(void)
{
	void *ptr;
	MBUF *mbuf_ptr;

	ptr = dave_malloc(1024);
	dave_free(ptr);
	dave_free(ptr);

	mbuf_ptr = dave_mmalloc(1024);
	dave_mfree(mbuf_ptr);
	dave_mfree(mbuf_ptr);
}

static void
_mem_test_memory_big_malloc(void)
{
	ub len = 1024 *1024;
	void *ptr = dave_malloc(len);

	MEMLOG("ptr:%lx len:%d", ptr, len);

	dave_free(ptr);
}

// =====================================================================

void
mem_test(void)
{
	_mem_test_memory_len();
	_mem_test_memory_mbuf();
	_mem_test_memory_overflow();
	_mem_test_memory_double_free();
	_mem_test_memory_big_malloc();
}

#endif

