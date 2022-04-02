/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_BSON_MEM_H__
#define __T_BSON_MEM_H__

tBsonObject * t_bson_object_malloc(void);
void t_bson_object_free(tBsonObject *pBson);

tBsonObject * t_bson_array_malloc(void);
void t_bson_array_free(tBsonObject *pBson);

tBsonData * t_bson_data_malloc(char *key_ptr, size_t key_len);
void t_bson_data_free(tBsonData *pData);

tBsonSerialize t_bson_serialize_malloc(tBsonObject *pBson);
void t_bson_serialize_free(tBsonObject *pBson);

tBsonData * t_bson_boolean_build(char *key_ptr, size_t key_len, bool value);
tBsonData * t_bson_int_build(char *key_ptr, size_t key_len, int value);
tBsonData * t_bson_int64_build(char *key_ptr, size_t key_len, u64 value);
tBsonData * t_bson_double_build(char *key_ptr, size_t key_len, double value);
tBsonData * t_bson_string_build(char *key_ptr, size_t key_len, char *value_ptr, size_t value_len);
tBsonData * t_bson_bin_build(char *key_ptr, size_t key_len, char *valur_ptr, size_t value_len);
tBsonData * t_bson_object_build(char *key_ptr, size_t key_len, tBsonObject *pBson);

#endif

