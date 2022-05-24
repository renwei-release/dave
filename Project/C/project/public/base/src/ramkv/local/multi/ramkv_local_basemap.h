/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_LOCAL_BASEMAP_H__
#define __RAMKV_LOCAL_BASEMAP_H__

KVLocalMultiBaseMap * ramkv_local_basemap_malloc(void *value_ptr, ub value_len);

void ramkv_local_basemap_free(KVLocalMultiBaseMap *pBaseMap);

KVLocalMultiBaseMap * ramkv_local_basemap_inq(KVLocalMultiBaseMap **ppBaseMap, u8 *key_ptr, ub key_len, dave_bool inq_for_add);

KVLocalMultiBaseMap * ramkv_local_basemap_add(KVLocalMultiBaseMap **ppBaseMap, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

KVLocalMultiBaseMap * ramkv_local_basemap_del(KVLocalMultiBaseMap **ppBaseMap, u8 *key_ptr, ub key_len);

void ramkv_local_basemap_clean(KVLocalMultiBaseMap *pBaseMap);

void ramkv_local_basemap_copy_value_from_user(KVLocalMultiBaseMap *pBaseMap, void *value_ptr, ub value_len);

ub ramkv_local_basemap_copy_value_to_user(KVLocalMultiBaseMap *pBaseMap, void *value_ptr, ub value_len);

#endif

