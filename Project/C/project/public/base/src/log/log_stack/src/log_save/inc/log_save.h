/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_SAVE_H__
#define __LOG_SAVE_H__

void log_save_init(ub log_reserved_days);

void log_save_exit(void);

void log_save_json_file(s8 *project_name, s8 *device_info, TraceLevel level, s8 *content_ptr, ub content_len);

void log_save_txt_file(s8 *project_name, s8 *device_info, TraceLevel level, s8 *content_ptr, ub content_len);

void log_save_chain_file(s8 *chain_name, s8 *device_info, s8 *service_verno, s8 *content_ptr, ub content_len);

#endif

