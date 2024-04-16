/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BDATA__
#include "dave_base.h"
#include "dave_bdata.h"

#define FILE_HOME_DIR (s8 *)"RECORDER"

// =====================================================================

s8 *
log_file_home_name(void)
{
	return FILE_HOME_DIR;
}

s8 *
log_req_to_log_file(s8 *file_ptr, ub file_len, BDataLogReq *pReq)
{
	if(pReq->sub_flag[0] == '\0')
	{
		dave_product(pReq->version, file_ptr, file_len);
	}
	else
	{
		s8 product[DAVE_VERNO_STR_LEN];

		dave_product(pReq->version, product, sizeof(product));

		dave_snprintf(file_ptr, file_len, "%s/%s", product, pReq->sub_flag);
	}

	return file_ptr;
}

s8 *
log_file_full_path(s8 *full_path, ub full_len, s8 *file_dir)
{
	dave_snprintf(full_path, full_len, "%s/%s/%s", dave_os_file_home_dir(), log_file_home_name(), file_dir);
	return full_path;
}

s8 *
log_req_to_full_path(s8 *full_path, ub full_len, BDataLogReq *pReq)
{
	DateStruct date;
	s8 file_dir[128];
	ub full_index;

	t_time_get_date(&date);

	log_req_to_log_file(file_dir, sizeof(file_dir), pReq);

	log_file_full_path(full_path, full_len, file_dir);

	full_index = dave_strlen(full_path);

	dave_snprintf(&full_path[full_index], full_len-full_index,
		"/%04d/%02d/%02d",
		date.year, date.month, date.day);

	return full_path;
}

#endif

