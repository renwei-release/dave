/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _T_RPC_VER3_METADATA_H__
#define _T_RPC_VER3_METADATA_H__

void t_rpc_ver3_metadata_work(dave_bool work_flag);

void * __t_rpc_ver3_zip_dave_bool__(dave_bool zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_dave_bool(zip_data) __t_rpc_ver3_zip_dave_bool__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_dave_bool__(dave_bool *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_dave_bool(unzip_data, pArrayJson) __t_rpc_ver3_unzip_dave_bool__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_char_d__(char *zip_data, ub zip_h_len, ub zip_l_len, s8 *fun, ub line);
#define t_rpc_ver3_zip_char_d(zip_data, zip_h_len, zip_l_len) __t_rpc_ver3_zip_char_d__(zip_data, zip_h_len, zip_l_len, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_char_d__(char *unzip_data, ub unzip_h_len, ub unzip_l_len, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_char_d(unzip_data, unzip_h_len, unzip_l_len, pArrayJson) __t_rpc_ver3_unzip_char_d__(unzip_data, unzip_h_len, unzip_l_len, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_int__(int zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_int(zip_data) __t_rpc_ver3_zip_int__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_int__(int *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_int(unzip_data, pArrayJson) __t_rpc_ver3_unzip_int__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_s8__(s8 zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_s8(zip_data) __t_rpc_ver3_zip_s8__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_s8__(s8 *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_s8(unzip_data, pArrayJson) __t_rpc_ver3_unzip_s8__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_s8_d__(s8 *zip_data, ub zip_h_len, ub zip_l_len, s8 *fun, ub line);
#define t_rpc_ver3_zip_s8_d(zip_data, zip_h_len, zip_l_len) __t_rpc_ver3_zip_s8_d__(zip_data, zip_h_len, zip_l_len, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_s8_d__(s8 *unzip_data, ub unzip_h_len, ub unzip_l_len, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_s8_d(unzip_data, unzip_h_len, unzip_l_len, pArrayJson) __t_rpc_ver3_unzip_s8_d__(unzip_data, unzip_h_len, unzip_l_len, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_u8__(u8 zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_u8(zip_data) __t_rpc_ver3_zip_u8__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_u8__(u8 *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_u8(unzip_data, pArrayJson) __t_rpc_ver3_unzip_u8__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_u8_d__(u8 *zip_data, ub zip_len, s8 *fun, ub line);
#define t_rpc_ver3_zip_u8_d(zip_data, zip_len) __t_rpc_ver3_zip_u8_d__(zip_data, zip_len, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_u8_d__(u8 *unzip_data, ub unzip_len, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_u8_d(unzip_data, unzip_len, pArrayJson) __t_rpc_ver3_unzip_u8_d__(unzip_data, unzip_len, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_s16__(s16 zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_s16(zip_data) __t_rpc_ver3_zip_s16__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_s16__(s16 *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_s16(unzip_data, pArrayJson) __t_rpc_ver3_unzip_s16__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_u16__(u16 zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_u16(zip_data) __t_rpc_ver3_zip_u16__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_u16__(u16 *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_u16(unzip_data, pArrayJson) __t_rpc_ver3_unzip_u16__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_s32__(s32 zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_s32(zip_data) __t_rpc_ver3_zip_s32__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_s32__(s32 *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_s32(unzip_data, pArrayJson) __t_rpc_ver3_unzip_s32__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_u32__(u32 zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_u32(zip_data) __t_rpc_ver3_zip_u32__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_u32__(u32 *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_u32(unzip_data, pArrayJson) __t_rpc_ver3_unzip_u32__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_s64__(s64 zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_s64(zip_data) __t_rpc_ver3_zip_s64__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_s64__(s64 *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_s64(unzip_data, pArrayJson) __t_rpc_ver3_unzip_s64__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_u64__(u64 zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_u64(zip_data) __t_rpc_ver3_zip_u64__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_u64__(u64 *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_u64(unzip_data, pArrayJson) __t_rpc_ver3_unzip_u64__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_sb__(sb zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_sb(zip_data) __t_rpc_ver3_zip_sb__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_sb__(sb *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_sb(unzip_data, pArrayJson) __t_rpc_ver3_unzip_sb__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_ub__(ub zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_ub(zip_data) __t_rpc_ver3_zip_ub__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_ub__(ub *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_ub(unzip_data, pArrayJson) __t_rpc_ver3_unzip_ub__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_ub_d__(ub *zip_data, ub zip_len, s8 *fun, ub line);
#define t_rpc_ver3_zip_ub_d(zip_data, zip_len) __t_rpc_ver3_zip_ub_d__(zip_data, zip_len, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_ub_d__(ub *unzip_data, ub unzip_len, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_ub_d(unzip_data, unzip_len, pArrayJson) __t_rpc_ver3_unzip_ub_d__(unzip_data, unzip_len, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_float__(float zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_float(zip_data) __t_rpc_ver3_zip_float__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_float__(float *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_float(unzip_data, pArrayJson) __t_rpc_ver3_unzip_float__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_float_d__(float *zip_data, ub zip_len, s8 *fun, ub line);
#define t_rpc_ver3_zip_float_d(zip_data, zip_len) __t_rpc_ver3_zip_float_d__(zip_data, zip_len, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_float_d__(float *unzip_data, ub unzip_len, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_float_d(unzip_data, unzip_len, pArrayJson) __t_rpc_ver3_unzip_float_d__(unzip_data, unzip_len, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_double__(double zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_double(zip_data) __t_rpc_ver3_zip_double__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_double__(double *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_double(unzip_data, pArrayJson) __t_rpc_ver3_unzip_double__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_double_d__(double *zip_data, ub zip_len, s8 *fun, ub line);
#define t_rpc_ver3_zip_double_d(zip_data, zip_len) __t_rpc_ver3_zip_double_d__(zip_data, zip_len, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_double_d__(double *unzip_data, ub unzip_len, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_double_d(unzip_data, unzip_len, pArrayJson) __t_rpc_ver3_unzip_double_d__(unzip_data, unzip_len, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_ThreadId__(ThreadId zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_ThreadId(zip_data) __t_rpc_ver3_zip_ThreadId__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_ThreadId__(ThreadId *unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_ThreadId(unzip_data, pArrayJson) __t_rpc_ver3_unzip_ThreadId__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_MBUF_ptr__(MBUF *zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_MBUF_ptr(zip_data) __t_rpc_ver3_zip_MBUF_ptr__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_MBUF_ptr__(MBUF **unzip_data, void *pJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_MBUF_ptr(unzip_data, pJson) __t_rpc_ver3_unzip_MBUF_ptr__(unzip_data, pJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_s8_ptr__(s8 *zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_s8_ptr(zip_data) __t_rpc_ver3_zip_s8_ptr__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_s8_ptr__(s8 **unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_s8_ptr(unzip_data, pArrayJson) __t_rpc_ver3_unzip_s8_ptr__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_u8_ptr__(u8 *zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_u8_ptr(zip_data) __t_rpc_ver3_zip_u8_ptr__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_u8_ptr__(u8 **unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_u8_ptr(unzip_data, pArrayJson) __t_rpc_ver3_unzip_u8_ptr__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_float_ptr__(float *zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_float_ptr(zip_data) __t_rpc_ver3_zip_float_ptr__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_float_ptr__(float **unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_float_ptr(unzip_data, pArrayJson) __t_rpc_ver3_unzip_float_ptr__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

void * __t_rpc_ver3_zip_void_ptr__(void *zip_data, s8 *fun, ub line);
#define t_rpc_ver3_zip_void_ptr(zip_data) __t_rpc_ver3_zip_void_ptr__(zip_data, (s8 *)__func__, (ub)__LINE__)
dave_bool __t_rpc_ver3_unzip_void_ptr__(void **unzip_data, void *pArrayJson, s8 *fun, ub line);
#define t_rpc_ver3_unzip_void_ptr(unzip_data, pArrayJson) __t_rpc_ver3_unzip_void_ptr__(unzip_data, pArrayJson, (s8 *)__func__, (ub)__LINE__)

#endif

