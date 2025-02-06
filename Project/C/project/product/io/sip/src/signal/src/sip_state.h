/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SIP_STATE_H__
#define __SIP_STATE_H__
#include "sip_signal.h"

void sip_state_init(void);
void sip_state_exit(void);

void sip_state_signal_creat(SIPSignal *pSignal);
void sip_state_signal_release(SIPSignal *pSignal);

void sip_state_call_creat(SIPCall *pCall);
void sip_state_call_release(SIPCall *pCall);

#endif

