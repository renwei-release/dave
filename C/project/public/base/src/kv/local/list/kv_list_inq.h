/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_LIST_INQ_H__
#define __KV_LIST_INQ_H__

ub __kv_list_inq__(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);

dave_bool __kv_list_top__(KV *pKV, u8 *key_ptr, ub key_len);

#endif

