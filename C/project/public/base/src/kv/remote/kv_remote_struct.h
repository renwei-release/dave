/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_REMOTE_STRUCT_H__
#define __KV_REMOTE_STRUCT_H__

dave_bool kv_malloc_remote(KV *pKV, s8 *name, KVAttrib attrib);

void kv_free_remote(KV *pKV, KVAttrib attrib);

#endif

