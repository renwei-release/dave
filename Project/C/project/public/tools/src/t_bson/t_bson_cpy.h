/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_BSON_CPY_H__
#define __T_BSON_CPY_H__

bool t_bson_string_cpy(tBsonObject *pBson, char *key_ptr, size_t key_len, char *pStringValue, size_t *pStringLen);
bool t_bson_bin_cpy(tBsonObject *pBson, char *key_ptr, size_t key_len, char *pBinValue, size_t *pBinLen);

bool t_bson_array_string_cpy(tBsonObject *pBson, size_t index, char *pStringValue, size_t *pStringLen);
bool t_bson_array_bin_cpy(tBsonObject *pBson, size_t index, char *pBinValue, size_t *pBinLen);
MBUF * t_bson_array_mbuf_cpy(tBsonObject *pBson, size_t index);

#endif

