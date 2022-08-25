/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_REMOTE_API_H__
#define __RAMKV_REMOTE_API_H__

dave_bool ramkv_remote_add(KvAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

sb ramkv_remote_inq(KvAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

ub ramkv_remote_del(KvAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len);

#endif

