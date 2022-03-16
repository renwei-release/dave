/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.01.25.
 * ================================================================================
 */

#ifndef __T_BSON_CPY_H__
#define __T_BSON_CPY_H__

bool t_bson_string_cpy(tBsonObject *pBson, char *key_ptr, size_t key_len, char *pStringValue, size_t *pStringLen);
bool t_bson_bin_cpy(tBsonObject *pBson, char *key_ptr, size_t key_len, char *pBinValue, size_t *pBinLen);

bool t_bson_array_string_cpy(tBsonObject *pBson, size_t index, char *pStringValue, size_t *pStringLen);
bool t_bson_array_bin_cpy(tBsonObject *pBson, size_t index, char *pBinValue, size_t *pBinLen);

#endif

