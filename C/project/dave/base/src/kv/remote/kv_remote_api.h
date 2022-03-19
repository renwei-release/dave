/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_REMOTE_API_H__
#define __KV_REMOTE_API_H__

dave_bool kv_remote_add(KVAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

ub kv_remote_inq(KVAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

ub kv_remote_del(KVAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len);

#endif

