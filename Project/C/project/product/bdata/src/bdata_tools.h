/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BDATA_TOOLS_H__
#define __BDATA_TOOLS_H__
#include "dave_base.h"

s8 * log_file_home_name(void);

s8 * log_req_to_log_file(s8 *file_ptr, ub file_len, BDataLogReq *pReq);

s8 * log_file_full_path(s8 *full_path, ub full_len, s8 *file_dir);

s8 * log_req_to_full_path(s8 *full_path, ub full_len, BDataLogReq *pReq);

#endif

