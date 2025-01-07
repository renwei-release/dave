/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SIP_GLOBAL_LOCK_H__
#define __SIP_GLOBAL_LOCK_H__

void sip_global_lock_init(void);
void sip_global_lock_exit(void);

void sip_global_lock(void);
void sip_global_unlock(void);

#endif

