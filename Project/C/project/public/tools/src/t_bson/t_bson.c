/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "t_bson_define.h"
#include "t_bson_mem.h"
#include "t_bson_add.h"
#include "t_bson_inq.h"
#include "t_bson_cpy.h"
#include "t_bson_serialize.h"
#include "t_bson_json.h"
#include "t_bson.h"

// =====================================================================

void *
t_bson_malloc_object(void)
{
	return (void *)t_bson_object_malloc();
}

void
t_bson_free_object(void *pBson)
{
	t_bson_object_free((tBsonObject *)pBson);
}

void *
t_bson_malloc_array(void)
{
	return (void *)t_bson_array_malloc();
}

void
t_bson_free_array(void *pBson)
{
	t_bson_array_free((tBsonObject *)pBson);
}

void
t_bson_add_boolean(void *pBson, char *key, bool value)
{
	t_bson_boolean_add((tBsonObject *)pBson, key, 0, value);
}

bool
t_bson_inq_boolean(void *pBson, char *key, bool *value)
{
	return t_bson_boolean_inq((tBsonObject *)pBson, key, 0, value);
}

void
t_bson_add_int(void *pBson, char *key, int value)
{
	t_bson_int_add((tBsonObject *)pBson, key, 0, value);
}

bool
t_bson_inq_int(void *pBson, char *key, int *value)
{
	return t_bson_int_inq((tBsonObject *)pBson, key, 0, value);
}

void
t_bson_add_int64(void *pBson, char *key, u64 value)
{
	t_bson_int64_add((tBsonObject *)pBson, key, 0, value);
}

bool
t_bson_inq_int64(void *pBson, char *key, u64 *value)
{
	return t_bson_int64_inq((tBsonObject *)pBson, key, 0, value);
}

void
t_bson_add_double(void *pBson, char *key, double value)
{
	t_bson_double_add((tBsonObject *)pBson, key, 0, value);
}

bool
t_bson_inq_double(void *pBson, char *key, double *double_value)
{
	return t_bson_double_inq((tBsonObject *)pBson, key, 0, double_value);
}

void
t_bson_add_string(void *pBson, char *key, char *value)
{
	t_bson_string_add((tBsonObject *)pBson, key, 0, value, 0);
}

bool
t_bson_inq_string(void *pBson, char *key, char **ppStringValue, size_t *pStringLen)
{
	return t_bson_string_inq((tBsonObject *)pBson, key, 0, ppStringValue, pStringLen);
}

bool
t_bson_cpy_string(void *pBson, char *key, char *pStringValue, size_t StringLen)
{
	return t_bson_string_cpy((tBsonObject *)pBson, key, 0, pStringValue, &StringLen);
}

void
t_bson_add_bin(void *pBson, char *key, char *value_ptr, size_t value_len)
{
	t_bson_bin_add((tBsonObject *)pBson, key, 0, value_ptr, value_len);
}

void
t_bson_ins_bin(void *pBson, char *key, char *value_ptr, size_t value_len)
{
	t_bson_bin_ins((tBsonObject *)pBson, key, 0, value_ptr, value_len);
}

bool
t_bson_inq_bin(void *pBson, char *key, char **ppBinValue, size_t *pBinLen)
{
	return t_bson_bin_inq((tBsonObject *)pBson, key, 0, ppBinValue, pBinLen);
}

bool
t_bson_cpy_bin(void *pBson, char *key, char *pBinValue, size_t *pBinLen)
{
	return t_bson_bin_cpy((tBsonObject *)pBson, key, 0, pBinValue, pBinLen);
}

void
t_bson_add_object(void *pBson, char *key, void *pAddBson)
{
	t_bson_object_add((tBsonObject *)pBson, key, 0, pAddBson);
}

void *
t_bson_inq_object(void *pBson, char *key)
{
	tBsonObject *pObject;

	if(t_bson_object_inq((tBsonObject *)pBson, key, 0, &pObject) == false)
		return NULL;

	return pObject;
}

void *
t_bson_clone_object(void *pBson, char *key)
{
	tBsonObject *pKeyObject;
	char *serialize_ptr;
	size_t serialize_len;

	pKeyObject = t_bson_inq_object(pBson, key);
	if(pKeyObject == NULL)
	{
		return NULL;
	}

	serialize_ptr = t_bson_to_serialize(pKeyObject, &serialize_len);

	return t_serialize_to_bson(serialize_ptr, serialize_len);
}

size_t
t_bson_array_number(void *pBson)
{
	return t_bson_array_number_inq((tBsonObject *)pBson);
}

void
t_bson_array_add_boolean(void *pBson, bool value)
{
	t_bson_array_boolean_add((tBsonObject *)pBson, value);
}

bool
t_bson_array_inq_boolean(void *pBson, size_t index, bool *bool_value)
{
	return t_bson_array_boolean_inq((tBsonObject *)pBson, index, bool_value);
}

void
t_bson_array_add_int(void *pBson, int value)
{
	t_bson_array_int_add((tBsonObject *)pBson, value);
}

bool
t_bson_array_inq_int(void *pBson, size_t index, int *value)
{
	return t_bson_array_int_inq((tBsonObject *)pBson, index, value);
}

void
t_bson_array_add_int64(void *pBson, u64 value)
{
	t_bson_array_int64_add((tBsonObject *)pBson, value);
}

bool
t_bson_array_inq_int64(void *pBson, size_t index, u64 *value)
{
	return t_bson_array_int64_inq((tBsonObject *)pBson, index, value);
}

void
t_bson_array_add_double(void *pBson, double value)
{
	t_bson_array_double_add((tBsonObject *)pBson, value);
}

bool
t_bson_array_inq_double(void *pBson, size_t index, double *double_value)
{
	return t_bson_array_double_inq((tBsonObject *)pBson, index, double_value);
}

void
t_bson_array_add_string(void *pBson, char *value)
{
	t_bson_array_string_add((tBsonObject *)pBson, value, 0);
}

bool
t_bson_array_inq_string(void *pBson, size_t index, char **ppStringValue, size_t *pStringLen)
{
	return t_bson_array_string_inq((tBsonObject *)pBson, index, ppStringValue, pStringLen);
}

bool
t_bson_array_cpy_string(void *pBson, size_t index, char *pStringValue, size_t *pStringLen)
{
	return t_bson_array_string_cpy((tBsonObject *)pBson, index, pStringValue, pStringLen);
}

void
t_bson_array_add_bin(void *pBson, char *value_ptr, size_t value_len)
{
	t_bson_array_bin_add((tBsonObject *)pBson, value_ptr, value_len);
}

void
t_bson_array_ins_bin(void *pBson, char *value_ptr, size_t value_len)
{
	t_bson_array_bin_ins((tBsonObject *)pBson, value_ptr, value_len);
}

bool
t_bson_array_inq_bin(void *pBson, size_t index, char **ppBinValue, size_t *pBinLen)
{
	return t_bson_array_bin_inq((tBsonObject *)pBson, index, ppBinValue, pBinLen);
}

bool
t_bson_array_cpy_bin(void *pBson, size_t index, char *pBinValue, size_t *pBinLen)
{
	return t_bson_array_bin_cpy((tBsonObject *)pBson, index, pBinValue, pBinLen);
}

void
__t_bson_array_add_mbuf__(void *pBson, MBUF *mbuf_data, s8 *fun, ub line)
{
	if(mbuf_data == NULL)
		return;

	__t_bson_array_mbuf_add__((tBsonObject *)pBson, mbuf_data, fun, line);
}

void
__t_bson_array_ins_mbuf__(void *pBson, MBUF *mbuf_data, s8 *fun, ub line)
{
	if(mbuf_data == NULL)
		return;

	__t_bson_array_mbuf_ins__((tBsonObject *)pBson, mbuf_data, fun, line);
}

MBUF *
t_bson_array_cpy_mbuf(void *pBson, size_t index)
{
	return t_bson_array_mbuf_cpy((tBsonObject *)pBson, index);
}

void
t_bson_array_add_object(void *pBson, void *pAddBson)
{
	t_bson_array_object_add((tBsonObject *)pBson, pAddBson);
}

void *
t_bson_array_inq_object(void *pBson, size_t index)
{
	tBsonObject *pObject;

	if(t_bson_array_object_inq((tBsonObject *)pBson, index, &pObject) == false)
		return NULL;

	return pObject;
}

char *
t_bson_to_serialize(void *pBson, size_t *serialize_len)
{
	tBsonSerialize serialize;

	serialize = t_bson_serialize_malloc(pBson);

	serialize.actual_len = t_bson_serialize((tBsonObject *)pBson, (unsigned char *)serialize.ptr, serialize.estimated_len);

	((tBsonObject *)pBson)->serialize = serialize;

	*serialize_len = serialize.actual_len;

	return serialize.ptr;
}

void *
t_serialize_to_bson(char *serialize_ptr, size_t serialize_len)
{
	return (void *)t_serialize_bson((unsigned char *)serialize_ptr, serialize_len);
}

MBUF *
t_bson_to_mbuf(void *pBson)
{
	tBsonSerialize serialize;
	MBUF *pMbuf;

	if(pBson == NULL)
		return NULL;

	serialize = ((tBsonObject *)pBson)->serialize;

	pMbuf = dave_mmalloc(serialize.estimated_len);

	pMbuf->len = pMbuf->tot_len = t_bson_serialize((tBsonObject *)pBson, dave_mptr(pMbuf), pMbuf->len);

	return pMbuf;
}

void *
t_mbuf_to_bson(MBUF *pMbuf)
{
	if(pMbuf == NULL)
		return NULL;

	return (void *)t_serialize_bson(dave_mptr(pMbuf), pMbuf->len);
}

void *
t_bson_to_json(void *pBson)
{
	return t_bson_json(pBson);
}

void *
t_json_to_bson(void *pJson)
{
	return t_json_bson(pJson);;
}

