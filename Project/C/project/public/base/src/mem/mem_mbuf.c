/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "dave_base.h"
#include "mem_lock.h"
#include "mem_log.h"

#define base_type ub
#ifdef PLATFORM_64_BIT
#define base_mask (0xfffffffffffffff8)
#define base_basis_margin (8)
#else
#define base_mask (0xfffffffc)
#define base_basis_margin (4)
#endif

// =====================================================================

MBUF *
__mbuf_mmalloc__(ub length, s8 *file, ub line)
{
	MBUF *m;

	m = (MBUF *)__base_malloc__((ub)(sizeof(MBUF)+base_basis_margin+length+1), dave_false, 0x00, file, line);
	if(m == NULL)
	{
		return NULL;
	}

	m->next = NULL;

	m->payload = (void *)(((ub)m+sizeof(MBUF)+base_basis_margin)&base_mask);
	m->tot_len = length;
	m->len = length;
	m->ref = 1;
	m->alloc_len = length;

	/*
	 * 在内存分配时已经默认多分配了一个字节位置
	 */
	((u8 *)(m->payload))[0] = '\0';
	((u8 *)(m->payload))[length] = '\0';

	return m;
}

sb
__mbuf_mheader__(MBUF *m, sb header_size_increment, s8 *file, ub line)
{
	if((m == NULL) || (header_size_increment == 0))
	{
		return 1;
	}

	m->payload = (u8 *)m->payload + header_size_increment;

	if(header_size_increment > 0)
	{
		if(m->len >= header_size_increment)
			m->len -= header_size_increment;
		if(m->tot_len >= header_size_increment)
			m->tot_len -= header_size_increment;
	}
	else
	{
		m->len -= header_size_increment;
		m->tot_len -= header_size_increment;		
	}

	return 0;
}

ub
__mbuf_mfree__(MBUF *m, s8 *file, ub line)
{
	MBUF *n;
	ub count;

	if(m == NULL)
	{
		return 0;
	}

	count = 0;

	while(m != NULL)
	{
		sb ref;

		mem_lock();
		ref = (sb)(-- (m->ref));
		mem_unlock();

		if(ref <= 0)
		{
			n = m->next;

			__base_free__(m, file, line);

			count ++;

			m = n;
		}
		else
		{
			m = NULL;
		}
	}

	return count;
}

ub
__mbuf_mclean__(MBUF *m, s8 *file, ub line)
{
	MBUF *n;
	ub count;

	if(m == NULL)
	{
		return 0;
	}

	count = 0;

	while(m != NULL)
	{
		n = m->next;

		__base_free__(m, file, line);

		count ++;

		m = n;
	}

	return count;
}

void
__mbuf_mref__(MBUF *m, s8 *file, ub line)
{
	if (m != NULL)
	{
		mem_lock();
		++ (m->ref);
		mem_unlock();
	}
    else
    {
        MEMTRACE(("%s:%d call failed!", file, line));
    }
}

MBUF *
__mbuf_mchain__(MBUF *cur_point, MBUF *cat_point, s8 *file, ub line)
{
	MBUF *p;

	if(cur_point == NULL)
	{
		return cat_point;
	}

	if(cat_point == NULL)
	{
		return cur_point;
	}

	if(cur_point == cat_point)
	{
		MEMABNOR("The behavior of loop addition was found at <%s:%d>", file, line);
		return cur_point;
	}

	for(p=cur_point; p->next!=NULL; p=p->next)
	{
		p->tot_len += cat_point->tot_len;
	}

	if((p->tot_len != p->len) || (p->next != NULL))
	{
		MEMTRACE(("%s:%d call mbuf_cat_test fail, (%x,%x,%x)",
			file, line, p->tot_len, p->len, p->next));
		return cur_point;
	}
	
	p->tot_len += cat_point->tot_len;
	p->next = cat_point;

	return cur_point;
}

MBUF *
__mbuf_mdechain__(MBUF *m, s8 *file, ub line)
{
	MBUF *n = NULL;

	if(m == NULL)
	{
		return NULL;
	}

	n = m->next;
	if (n != NULL)
	{
		if(n->tot_len != (m->tot_len - m->len))
		{
			MEMTRACE(("%s:%d call mbuf_dechain_test fail, (%x,%x,%x)", file, line, n->tot_len, m->tot_len, m->len));
		}
		m->next = NULL;
		m->tot_len = m->len;
	}

	return n;
}

MBUF *
__mbuf_clone__(MBUF *me, s8 *file, ub line)
{
	MBUF *clone, *new_mbuf;

	if(me == NULL)
	{
		return NULL;
	}

	clone = NULL;

	while(me != NULL)
	{
		new_mbuf = __base_mmalloc__(me->len + 1, file, line);

		new_mbuf->tot_len = new_mbuf->len = me->len;

		dave_memcpy(new_mbuf->payload, me->payload, me->len);

		((u8 *)new_mbuf->payload)[me->len] = '\0';

		clone = __base_mchain__(clone, new_mbuf, file, line);

		me = me->next;
	}

	return clone;
}

ub
__mbuf_list_number__(MBUF *m)
{
	ub list_number = 0;

	if(m == NULL)
	{
		return 0;
	}

	while(m != NULL)
	{
		list_number ++;

		m = m->next;
	}

	return list_number;
}

#endif

