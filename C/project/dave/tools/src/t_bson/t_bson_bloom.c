/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.02.22.
 * ================================================================================
 */
#include "dave_tools.h"
#include "t_bson_define.h"
#include "t_bson_inq.h"
#include "tools_log.h"

// =====================================================================

u64
t_bson_bloom(char *data_ptr, size_t data_len)
{
	u64 *u64_data_ptr = (u64 *)data_ptr;
	u64 bloom_value = 0;

	while(data_len >= 8)
	{
		bloom_value += *(u64_data_ptr ++);
		data_len -= 8;
	}

	return (0x1 << (bloom_value % 64));
}

