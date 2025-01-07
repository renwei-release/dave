/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SIP_CALL_CLASS_H__
#define __SIP_CALL_CLASS_H__
#include "dave_base.h"
#include "t_lock.h"
#include "dave_osip.h"
#include "sip_signal.h"

SIPCall * sip_call_build(SIPSignal *pSignal, s8 *call_data);

SIPCall * sip_call_id_query(SIPSignal *pSignal, s8 *call_id);

SIPCall * sip_call_data_query(SIPSignal *pSignal, s8 *call_data);

void sip_call_creat(SIPSignal *pSignal, s8 *call_id, SIPCall *pCall);

void sip_call_release(SIPSignal *pSignal, SIPCall *pCall);

#endif

