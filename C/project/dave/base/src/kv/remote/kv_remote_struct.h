/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_REMOTE_STRUCT_H__
#define __KV_REMOTE_STRUCT_H__

dave_bool kv_malloc_remote(KV *pKV, s8 *name, KVAttrib attrib);

void kv_free_remote(KV *pKV, KVAttrib attrib);

#endif

