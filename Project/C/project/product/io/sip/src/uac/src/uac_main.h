/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_MAIN_H__
#define __UAC_MAIN_H__
#include "dave_rtp.h"

void uac_main_init(void);
void uac_main_exit(void);

typedef struct {
	ThreadId owner_id;
	SIPCall *call;
} UACCall;

typedef struct {
	void *phone_number_kv;
	SIPSignal *signal;
} UACClass;

SIPSignal * uac_main_signal(void);

UACCall * uac_main_build_call(ThreadId owner_id, s8 *phone_number);

void uac_main_setup_call(UACCall *pUACCall, SIPCall *pCall);

UACCall * uac_main_inq_phone_number(s8 *phone_number);

UACCall * uac_main_inq_call_id(s8 *call_id);

void uac_main_del_call(s8 *phone_number);

#endif

