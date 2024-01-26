/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RECORDER_ALIYUN_H__
#define __RECORDER_ALIYUN_H__

dave_bool aliyun_log_init(void);

void aliyun_log_exit(void);

dave_bool aliyun_log_string(s8 *log_file, s8 *log_ptr, ub log_len);

dave_bool aliyun_log_json(s8 *log_file, void *pJson);

#endif

