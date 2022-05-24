/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_ERROR_CODE_H__
#define __DAVE_ERROR_CODE_H__

typedef enum {
	ERRCODE_OK = 0,
	ERRCODE_Limited_resources = -2,
	ERRCODE_Arithmetic_error = -3,
	ERRCODE_Invalid_data = -8,
	ERRCODE_Unsupported_type = -9,
	ERRCODE_save_failed = -16,
	ERRCODE_Invalid_parameter = - 20,
	ERRCODE_Request_failed = -26,
	ERRCODE_timer_out = -38,
	ERRCODE_invalid_type = -39,
	ERRCODE_ptr_null = -43,
	ERRCODE_invalid_option = -53,
	ERRCODE_lost_uuid = -69,
	ERRCODE_mismatch = -98,
	ERRCODE_decode_failed = -104,
	ERRCODE_file_open_failed = -107,
	ERRCODE_Failed_to_identify = -228,
	ERRCODE_invalid_content = -158,
	ERRCODE_empty_data = -186,
	ERRCODE_invalid_mcard = -190,
	ERRCODE_system_not_ready = -216,
	ERRCODE_wait_more_data = -223,
	ERRCODE_Failed_to_features = -229,
	ERRCODE_predict_failed = -240,
	ErrCode_max = 0x1fffffffffffffff
} ErrCode;

#endif

