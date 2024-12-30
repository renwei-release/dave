/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_STATE_H__
#define __UAC_STATE_H__
#include "dave_osip.h"
#include "uac_rtp.h"

void uac_state_init(void);
void uac_state_exit(void);

void uac_state_creat(UACClass *pClass);
void uac_state_release(UACClass *pClass);

#endif

