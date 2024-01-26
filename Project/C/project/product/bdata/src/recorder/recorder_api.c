/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BDATA__
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_base.h"
#include "recorder_file.h"
#include "recorder_aliyun.h"
#include "bdata_log.h"

// ====================================================================

void
recorder_api_init(void)
{
	recorder_file_init();
	aliyun_log_init();
}

void
recorder_api_exit(void)
{
	recorder_file_exit();
	aliyun_log_exit();
}

ub
recorder_api_info(s8 *info_ptr, ub info_len)
{
	return recorder_file_info(info_ptr, info_len);
}

#endif

