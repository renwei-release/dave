/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RECORDER_API_H__
#define __RECORDER_API_H__

void recorder_api_init(void);

void recorder_api_exit(void);

ub recorder_api_info(s8 *info_ptr, ub info_len);

#endif

