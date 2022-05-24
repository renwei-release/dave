/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_REMOTE_STRUCT_H__
#define __RAMKV_REMOTE_STRUCT_H__

dave_bool ramkv_malloc_remote(KV *pKV, s8 *name, KvAttrib attrib);

void ramkv_free_remote(KV *pKV, KvAttrib attrib);

#endif

