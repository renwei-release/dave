/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_SAVE_CHAIN_H__
#define __LOG_SAVE_CHAIN_H__

void log_save_chain_init(void);

void log_save_chain_exit(void);

void log_save_chain(sb file_id, s8 *device_info, s8 *service_verno, s8 *content_ptr, ub content_len);

#endif

