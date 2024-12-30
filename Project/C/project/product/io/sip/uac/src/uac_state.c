/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "uac_class.h"
#include "uac_reg.h"
#include "uac_global_lock.h"
#include "uac_log.h"

static void *_uac_state_kv = NULL;

static void
_uac_state_resend_request(UACClass *pClass)
{
	if((pClass->signal.get_register_request_intermediate_state == dave_false) && (pClass->signal.register_request != NULL))
	{
		UACLOG("auto resend register request!");

		uac_class_send(pClass, pClass->signal.register_request);
	}

	if((pClass->signal.get_invite_request_intermediate_state == dave_false) && (pClass->signal.invite_request != NULL))
	{
		UACLOG("auto resend invite request!");

		uac_class_send(pClass, pClass->signal.invite_request);
	}

	if((pClass->signal.get_bye_request_intermediate_state == dave_false) && (pClass->signal.bye_request != NULL))
	{
		UACLOG("auto resend bye request!");

		uac_class_send(pClass, pClass->signal.bye_request);
	}
}

static void
_uac_state_rereg(UACClass *pClass)
{
	if((++ pClass->signal.reg_timer_counter) > 90)
	{
		UACLOG("re register");

		pClass->signal.reg_timer_counter = 0;
		uac_reg(pClass);
	}
}

static void
_uac_state_kv_timer(void *ramkv, s8 *key)
{
	UACClass *pClass = kv_inq_key_ptr(ramkv, key);

	uac_global_lock();

	_uac_state_resend_request(pClass);
	_uac_state_rereg(pClass);

	uac_global_unlock();
}

// =====================================================================

void
uac_state_init(void)
{
	_uac_state_kv = kv_malloc("uackv", 3, _uac_state_kv_timer);
}

void
uac_state_exit(void)
{
	if(_uac_state_kv != NULL)
	{
		kv_free(_uac_state_kv, NULL);
		_uac_state_kv = NULL;
	}
}

void
uac_state_creat(UACClass *pClass)
{
	kv_add_ub_ptr(_uac_state_kv, (ub)pClass, pClass);
}

void
uac_state_release(UACClass *pClass)
{
	kv_del_ub_ptr(_uac_state_kv, (ub)pClass);
}

