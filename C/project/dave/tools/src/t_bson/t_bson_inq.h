/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.01.24.
 * ================================================================================
 */

#ifndef __T_BSON_INQ_H__
#define __T_BSON_INQ_H__

bool t_bson_boolean_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, bool *value);
bool t_bson_int_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, int *value);
bool t_bson_int64_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, u64 *value);
bool t_bson_double_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, double *value);
bool t_bson_string_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, char **ppStringValue, size_t *pStringLen);
bool t_bson_bin_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, char **ppBinValue, size_t *pBinLen);
bool t_bson_object_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, tBsonObject **ppObject);

size_t t_bson_array_number_inq(tBsonObject *pBson);
bool t_bson_array_boolean_inq(tBsonObject *pBson, size_t index, bool *value);
bool t_bson_array_int_inq(tBsonObject *pBson, size_t index, int *value);
bool t_bson_array_int64_inq(tBsonObject *pBson, size_t index, u64 *value);
bool t_bson_array_double_inq(tBsonObject *pBson, size_t index, double *value);
bool t_bson_array_string_inq(tBsonObject *pBson, size_t index, char **ppStringValue, size_t *pStringLen);
bool t_bson_array_bin_inq(tBsonObject *pBson, size_t index, char **ppBinValue, size_t *pBinLen);
bool t_bson_array_object_inq(tBsonObject *pBson, size_t index, tBsonObject **ppObject);

#endif

