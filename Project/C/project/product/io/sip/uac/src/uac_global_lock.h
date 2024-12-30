/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_GLOBAL_LOCK_H__
#define __UAC_GLOBAL_LOCK_H__
#include "dave_osip.h"
#include "uac_rtp.h"

void uac_global_lock_init(void);
void uac_global_lock_exit(void);

void uac_global_lock(void);
void uac_global_unlock(void);

#endif

