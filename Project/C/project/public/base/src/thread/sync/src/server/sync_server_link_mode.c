/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_base.h"
#include "dave_tools.h"
#include "sync_server_link_mode.h"

static void *_link_mode_kv = NULL;

static void
_sync_server_link_mode_add(dave_bool model, s8 *ptr_stra, s8 *ptr_strb)
{
	if(model == dave_false)
	{
		kv_add_key_ptr(_link_mode_kv, ptr_stra, (void *)MAIN_MODEL);
		kv_add_key_ptr(_link_mode_kv, ptr_strb, (void *)SUB_MODEL);
	}
	else
	{
		kv_add_key_ptr(_link_mode_kv, ptr_stra, (void *)SUB_MODEL);
		kv_add_key_ptr(_link_mode_kv, ptr_strb, (void *)MAIN_MODEL);
	}
}

static dave_bool
_sync_server_link_mode_arbitration(s8 *ptr_stra, s8 *ptr_strb, ub expected_mode)
{
	dave_bool model;

	if(expected_mode == NULL_MODEL)
	{
		model = ((t_rand() % 2) == 0) ? dave_true : dave_false;
	}
	else if(expected_mode == MAIN_MODEL)
	{
		model = dave_true;
	}
	else if(expected_mode == SUB_MODEL)
	{
		model = dave_false;
	}
	else
	{
		model = ((t_rand() % 2) == 0) ? dave_true : dave_false;
	}

	_sync_server_link_mode_add(model, ptr_stra, ptr_strb);

	return model;
}

static ub
_sync_server_link_mode_inq(s8 *ptr_stra, s8 *ptr_strb)
{
	void *ptr_stra_value, *ptr_strb_value;

	ptr_stra_value = kv_inq_key_ptr(_link_mode_kv, ptr_stra);
	if(ptr_stra_value == NULL)
		return NULL_MODEL;

	ptr_strb_value = kv_inq_key_ptr(_link_mode_kv, ptr_strb);
	if(ptr_strb_value == NULL)
		return NULL_MODEL;

	if(ptr_stra_value == ptr_strb_value)
		return NULL_MODEL;

	return (ub)ptr_stra_value;
}

// =====================================================================

void
sync_server_link_mode_init(void)
{
	_link_mode_kv = kv_malloc("lmkv", 0, NULL);
}

void
sync_server_link_mode_exit(void)
{
	kv_free(_link_mode_kv, NULL);
}

dave_bool
sync_server_link_mode(s8 *ptr_a, s8 *ptr_b, ub expected_mode)
{
	s8 ptr_string_a[128];
	s8 ptr_string_b[128];
	dave_bool ret;

	dave_snprintf(ptr_string_a, sizeof(ptr_string_a), "%s-%s", ptr_a, ptr_b);
	dave_snprintf(ptr_string_b, sizeof(ptr_string_b), "%s-%s", ptr_b, ptr_a);

	switch(_sync_server_link_mode_inq(ptr_string_a, ptr_string_b))
	{
		case MAIN_MODEL:
				ret = dave_true;
			break;
		case SUB_MODEL:
				ret = dave_false;
			break;
		default:
				ret = _sync_server_link_mode_arbitration(ptr_string_a, ptr_string_b, expected_mode);
			break;
	}

	return ret;
}

#endif

