/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_CFG_H__
#define __THREAD_CFG_H__

void thread_cfg_init(void);

void thread_cfg_exit(void);

ub thread_cfg_system_memory_max_use_percentage(void);

#endif

