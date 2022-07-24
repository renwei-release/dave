/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_SAVE_H__
#define __LOG_SAVE_H__

void log_save_init(void);

void log_save_exit(void);

void log_save_log_file(s8 *file_name, TraceLevel level, s8 *content_ptr, ub content_len);

void log_save_chain_file(s8 *file_name, s8 *device_info, s8 *content_ptr, ub content_len);

#endif

