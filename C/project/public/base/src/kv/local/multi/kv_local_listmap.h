/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_LOCAL_LISTMAP_H__
#define __KV_LOCAL_LISTMAP_H__

void kv_local_listmap_add(KVLocalMultiMap *pMultiMap, KVLocalMultiBaseMap *pBaseMap, s8 *fun, ub line);

ub kv_local_listmap_inq(KVLocalMultiMap *pMultiMap, sb index, void *value_ptr, ub value_len);

void kv_local_listmap_del(KVLocalMultiMap *pMultiMap, KVLocalMultiBaseMap *pBaseMap);

#endif
