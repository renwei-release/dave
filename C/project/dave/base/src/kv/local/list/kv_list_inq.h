/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_LIST_INQ_H__
#define __KV_LIST_INQ_H__

ub __kv_list_inq__(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);

dave_bool __kv_list_top__(KV *pKV, u8 *key_ptr, ub key_len);

#endif

