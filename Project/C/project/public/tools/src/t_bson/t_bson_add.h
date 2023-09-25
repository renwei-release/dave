/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_BSON_ADD_H__
#define __T_BSON_ADD_H__

void t_bson_boolean_add(tBsonObject *pBson, char *key_ptr, size_t key_len, bool value);
void t_bson_int_add(tBsonObject *pBson, char *key_ptr, size_t key_len, int value);
void t_bson_int64_add(tBsonObject *pBson, char *key_ptr, size_t key_len, u64 value);
void t_bson_double_add(tBsonObject *pBson, char *key_ptr, size_t key_len, double value);
void t_bson_string_add(tBsonObject *pBson, char *key_ptr, size_t key_len, char *valur_ptr, size_t value_len);
void t_bson_bin_add(tBsonObject *pBson, char *key_ptr, size_t key_len, char *valur_ptr, size_t value_len);
void t_bson_bin_ins(tBsonObject *pBson, char *key_ptr, size_t key_len, char *valur_ptr, size_t value_len);
void t_bson_object_add(tBsonObject *pBson, char *key_ptr, size_t key_len, tBsonObject *pAddBson);

void t_bson_array_boolean_add(tBsonObject *pBson, bool value);
void t_bson_array_int_add(tBsonObject *pBson, int value);
void t_bson_array_int64_add(tBsonObject *pBson, u64 value);
void t_bson_array_double_add(tBsonObject *pBson, double value);
void t_bson_array_string_add(tBsonObject *pBson, char *valur_ptr, size_t value_len);
void t_bson_array_bin_add(tBsonObject *pBson, char *valur_ptr, size_t value_len);
void t_bson_array_bin_ins(tBsonObject *pBson, char *valur_ptr, size_t value_len);
void __t_bson_array_mbuf_add__(tBsonObject *pBson, MBUF *mbuf_data, s8 *fun, ub line);
#define t_bson_array_mbuf_add(pBson, mbuf_data) __t_bson_array_mbuf_add__(pBson, mbuf_data, (s8 *)__func__, (ub)__LINE__)
void __t_bson_array_mbuf_ins__(tBsonObject *pBson, MBUF *mbuf_data, s8 *fun, ub line);
#define t_bson_array_mbuf_ins(pBson, mbuf_data) __t_bson_array_mbuf_ins__(pBson, mbuf_data, (s8 *)__func__, (ub)__LINE__)
void t_bson_array_object_add(tBsonObject *pBson, tBsonObject *pAddBson);

#endif

