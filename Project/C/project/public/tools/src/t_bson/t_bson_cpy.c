/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include "dave_tools.h"
#include "t_bson_define.h"
#include "t_bson_inq.h"
#include "tools_log.h"

// =====================================================================

bool
t_bson_string_cpy(tBsonObject *pBson, char *key_ptr, size_t key_len, char *pStringValue, size_t *pStringLen)
{
	char *string_cpy_ptr;
	size_t string_cpy_len;

	if(*pStringLen == 0)
		return false;

	if(t_bson_string_inq(pBson, key_ptr, key_len, &string_cpy_ptr, &string_cpy_len) == false)
	{
		pStringValue[0] = '\0';
		*pStringLen = 0;
		return false;
	}

	(*pStringLen) -= 1;

	if(*pStringLen <= string_cpy_len)
	{
		string_cpy_len = *pStringLen;
	}

	*pStringLen = dave_memcpy(pStringValue, string_cpy_ptr, string_cpy_len);

	pStringValue[string_cpy_len] = '\0';

	return true;
}

bool
t_bson_bin_cpy(tBsonObject *pBson, char *key_ptr, size_t key_len, char *pBinValue, size_t *pBinLen)
{
	char *bin_cpy_ptr;
	size_t bin_cpy_len;

	if(t_bson_bin_inq(pBson, key_ptr, key_len, &bin_cpy_ptr, &bin_cpy_len) == false)
	{
		pBinValue[0] = '\0';
		*pBinLen = 0;
		return false;
	}

	if(*pBinLen > bin_cpy_len)
	{
		pBinValue[bin_cpy_len] = '\0';
	}
	else
	{
		bin_cpy_len = *pBinLen;
	}

	*pBinLen = dave_memcpy(pBinValue, bin_cpy_ptr, bin_cpy_len);

	return true;
}

bool
t_bson_array_string_cpy(tBsonObject *pBson, size_t index, char *pStringValue, size_t *pStringLen)
{
	char *string_cpy_ptr;
	size_t string_cpy_len;

	if(t_bson_array_string_inq(pBson, index, &string_cpy_ptr, &string_cpy_len) == false)
	{
		pStringValue[0] = '\0';
		*pStringLen = 0;
		return false;
	}

	if(*pStringLen > string_cpy_len)
	{
		pStringValue[string_cpy_len] = '\0';
	}

	*pStringLen = dave_memcpy(pStringValue, string_cpy_ptr, string_cpy_len);

	return true;
}

bool
t_bson_array_bin_cpy(tBsonObject *pBson, size_t index, char *pBinValue, size_t *pBinLen)
{
	char *bin_cpy_ptr;
	size_t bin_cpy_len;

	if(t_bson_array_bin_inq(pBson, index, &bin_cpy_ptr, &bin_cpy_len) == false)
	{
		pBinValue[0] = '\0';
		*pBinLen = 0;
		return false;
	}

	if(*pBinLen > bin_cpy_len)
	{
		pBinValue[bin_cpy_len] = '\0';
	}

	*pBinLen = dave_memcpy(pBinValue, bin_cpy_ptr, bin_cpy_len);

	return true;
}

MBUF *
t_bson_array_mbuf_cpy(tBsonObject *pBson, size_t index)
{
	char *bin_cpy_ptr;
	size_t bin_cpy_len;

	if(t_bson_array_bin_inq(pBson, index, &bin_cpy_ptr, &bin_cpy_len) == false)
	{
		return NULL;
	}

	return t_a2b_bin_to_mbuf(bin_cpy_ptr, bin_cpy_len);
}

