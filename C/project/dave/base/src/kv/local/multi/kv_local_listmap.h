/*
 * ================================================================================
 * (c) Copyright 2021 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.01.21.
 * ================================================================================
 */

#ifndef __KV_LOCAL_LISTMAP_H__
#define __KV_LOCAL_LISTMAP_H__

void kv_local_listmap_add(KVLocalMultiMap *pMultiMap, KVLocalMultiBaseMap *pBaseMap, s8 *fun, ub line);

ub kv_local_listmap_inq(KVLocalMultiMap *pMultiMap, sb index, void *value_ptr, ub value_len);

void kv_local_listmap_del(KVLocalMultiMap *pMultiMap, KVLocalMultiBaseMap *pBaseMap);

#endif

