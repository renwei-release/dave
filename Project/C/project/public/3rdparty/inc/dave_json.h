/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_JSON_H__
#define __DAVE_JSON_H__

#include "json.h"

s8 * dave_json_c_version(void);
#define dave_json_version dave_json_c_version

void * __dave_string_to_json_c__(s8 *string_data, sb string_length, s8 *fun, ub line);
#define dave_string_to_json(string_data, string_length) __dave_string_to_json_c__((s8 *)string_data, (sb)string_length, (s8 *)__func__, (ub)__LINE__)

s8 * dave_json_c_to_string(void *object, ub *length);
#define dave_json_to_string dave_json_c_to_string

void * dave_json_c_malloc(void);
#define dave_json_malloc dave_json_c_malloc

void dave_json_c_free(void *object);
#define dave_json_free dave_json_c_free

void * dave_json_c_array_malloc(void);
#define dave_json_array_malloc dave_json_c_array_malloc

void dave_json_c_del_object(void *object, char *key);
#define dave_json_del_object dave_json_c_del_object

dave_bool dave_json_c_add_object(void *object, char *key, void *obj);
#define dave_json_add_object dave_json_c_add_object

void * dave_json_c_get_object(void *object, char *key);
#define dave_json_get_object dave_json_c_get_object

dave_bool dave_json_c_add_array(void *object, char *key, void *array);
#define dave_json_add_array dave_json_c_add_array

void * dave_json_c_get_array(void *object, char *key);
#define dave_json_get_array dave_json_c_get_array

dave_bool dave_array_add_json_c(void *array, void *object);
#define dave_array_add_json dave_array_add_json_c

dave_bool dave_json_c_add_ub(void *object, char *key, ub ub_data);
#define dave_json_add_ub dave_json_c_add_ub

dave_bool __dave_json_c_get_ub__(void *object, char *key, ub *ub_data, s8 *fun, ub line);
#define dave_json_get_ub(object, key, ub_data) __dave_json_c_get_ub__(object, (char *)key, ub_data, (s8 *)__func__, (ub)__LINE__)

dave_bool dave_json_c_add_sb(void *object, char *key, sb sb_data);
#define dave_json_add_sb dave_json_c_add_sb

dave_bool dave_json_c_get_sb(void *object, char *key, sb *sb_data);
#define dave_json_get_sb dave_json_c_get_sb

dave_bool dave_json_c_add_double(void *object, char *key, double double_data);
#define dave_json_add_double dave_json_c_add_double

dave_bool __dave_json_c_get_double__(void *object, char *key, double *double_data, s8 *fun, ub line);
#define dave_json_get_double(object, key, double_data) __dave_json_c_get_double__(object, key, double_data, (s8 *)__func__, (ub)__LINE__)

dave_bool dave_json_c_add_str(void *object, char *key, s8 *str_data);
#define dave_json_add_str dave_json_c_add_str

dave_bool dave_json_c_add_str_len(void *object, char *key, s8 *str_data, ub str_len);
#define dave_json_add_str_len dave_json_c_add_str_len

dave_bool __dave_json_c_get_str__(void *object, char *key, s8 *str_data, ub *str_length, s8 *fun, ub line);
#define dave_json_get_str(object, key, str_data, str_length) __dave_json_c_get_str__(object, (char *)key, (s8 *)str_data, str_length, (s8 *)__func__, (ub)__LINE__)

ub __dave_json_c_get_str_v2__(void *object, char *key, s8 *str_data, ub str_length, s8 *fun, ub line);
#define dave_json_get_str_v2(object, key, str_data, str_length) __dave_json_c_get_str_v2__(object, (char *)key, (s8 *)str_data, str_length, (s8 *)__func__, (ub)__LINE__)

ub __dave_json_c_get_str_length__(void *object, char *key, s8 *fun, ub line);
#define dave_json_get_str_length(object, key) __dave_json_c_get_str_length__(object, key, (s8 *)__func__, (ub)__LINE__)

dave_bool dave_json_c_add_boolean(void *object, char *key, dave_bool bool_data);
#define dave_json_add_boolean dave_json_c_add_boolean

dave_bool dave_json_c_get_boolean(void *object, char *key, dave_bool *bool_data);
#define dave_json_get_boolean dave_json_c_get_boolean

sb dave_json_c_get_array_length(void *array);
#define dave_json_get_array_length dave_json_c_get_array_length

void * dave_json_c_get_array_idx(void *array, sb index);
#define dave_json_get_array_idx dave_json_c_get_array_idx

dave_bool dave_json_c_array_add_str(void *array, s8 *str_data);
#define dave_json_array_add_str dave_json_c_array_add_str

dave_bool dave_json_c_array_add_str_len(void *array, s8 *str_data, ub str_len);
#define dave_json_array_add_str_len dave_json_c_array_add_str_len

s8 * dave_json_c_array_get_str(void *array, sb index, ub *string_len);
#define dave_json_array_get_str dave_json_c_array_get_str

dave_bool dave_json_c_array_add_sb(void *array, sb sb_data);
#define dave_json_array_add_sb dave_json_c_array_add_sb

sb dave_json_c_array_get_sb(void *array, sb index);
#define dave_json_array_get_sb dave_json_c_array_get_sb

dave_bool dave_json_c_array_add_double(void *array, double double_data);
#define dave_json_array_add_double dave_json_c_array_add_double

double dave_json_c_array_get_double(void *array, sb index);
#define dave_json_array_get_double dave_json_c_array_get_double

dave_bool dave_json_c_array_add_int(void *array, ub int_data);
#define dave_json_array_add_int dave_json_c_array_add_int

void * dave_json_c_array_get_object(void *array, sb index);
#define dave_json_array_get_object dave_json_c_array_get_object

dave_bool __dave_json_c_array_add_object__(void *array, void *pObject, s8 *fun, ub line);
#define dave_json_array_add_object(array, pObject) __dave_json_c_array_add_object__(array, pObject, (s8 *)__func__, (ub)__LINE__)

dave_bool __dave_json_c_array_del_object__(void *array, ub index, s8 *fun, ub line);
#define dave_json_array_del_object(array, index) __dave_json_c_array_del_object__(array, index, (s8 *)__func__, (ub)__LINE__)

dave_bool dave_json_c_array_add_array(void *array_dst, void *array_src);
#define dave_json_array_add_array dave_json_c_array_add_array

dave_bool dave_json_c_bool_copy(void *pDstJson, void *pSrcJson, char *key);
#define dave_json_bool_copy dave_json_c_bool_copy

dave_bool dave_json_c_ub_copy(void *pDstJson, void *pSrcJson, char *key);
#define dave_json_ub_copy dave_json_c_ub_copy

dave_bool dave_json_c_str_copy(void *pDstJson, void *pSrcJson, char *key);
#define dave_json_str_copy dave_json_c_str_copy

dave_bool dave_json_c_array_copy(void *pDstJson, void *pSrcJson, char *key);
#define dave_json_array_copy dave_json_c_array_copy

MBUF * dave_json_c_to_mbuf(void *pJson);
#define dave_json_to_mbuf dave_json_c_to_mbuf

void * dave_json_c_clone(void *pJson);
#define dave_json_clone dave_json_c_clone

dave_bool dave_json_c_write(void *pJson, s8 *file_name, dave_bool direct_flag);
#define dave_json_write dave_json_c_write

void * dave_json_c_read(s8 *file_name, dave_bool direct_flag);
#define dave_json_read dave_json_c_read

#endif

