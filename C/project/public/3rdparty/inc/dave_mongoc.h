/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_MONGOC_H__
#define __DAVE_MONGOC_H__

#define DAVE_NOSQL_KEY_MAX (12)

dave_bool dave_mongoc_init(dave_bool server_flag, ub port, s8 *db_name, s8 *user, s8 *password);

void dave_mongoc_exit(dave_bool server_flag);

void * dave_mongoc_connect(s8 *ip_addr, ub port, s8* db_name, s8 *user, s8 *password, void **ppDatabase);

void dave_mongoc_disconnect(void *pClient, void *pDatabase);

void * dave_mongoc_capture_collection(void *pClient, s8 *db_name, s8 *coll_name);

void dave_mongoc_release_collection(void *pCollection);

dave_bool __dave_mongoc_add_bin__(void *pCollection, s8 *obj_str, u8 *value, ub value_len, s8 *fun, ub line);
#define dave_mongoc_add_bin(pCollection, obj_str, value, value_len) __dave_mongoc_add_bin__(pCollection, obj_str, value, value_len, (s8 *)__func__, (ub)__LINE__)

dave_bool __dave_mongoc_upd_bin__(void *pCollection, s8 *obj_str, u8 *value, ub value_len, s8 *fun, ub line);
#define dave_mongoc_upd_bin(pCollection, obj_str, value, value_len) __dave_mongoc_upd_bin__(pCollection, obj_str, value, value_len, (s8 *)__func__, (ub)__LINE__)

dave_bool __dave_mongoc_det_bin__(void *pCollection, s8 *obj_str, s8 *fun, ub line);
#define dave_mongoc_det_bin(pCollection, obj_str) __dave_mongoc_det_bin__(pCollection, obj_str, (s8 *)__func__, (ub)__LINE__)

dave_bool __dave_mongoc_inq_bin__(void *pCollection, s8 *obj_str, u8 *value, ub *value_len, s8 *fun, ub line);
#define dave_mongoc_inq_bin(pCollection, obj_str, value, value_len) __dave_mongoc_inq_bin__(pCollection, obj_str, value, value_len, (s8 *)__func__, (ub)__LINE__)

dave_bool __dave_mongoc_add_json__(void *pCollection, s8 *pJson_string, s8 *fun, ub line);
#define dave_mongoc_add_json(pCollection, pJson_string) __dave_mongoc_add_json__(pCollection, pJson_string, (s8 *)__func__, (ub)__LINE__)

dave_bool __dave_mongoc_upd_json__(void *pCollection, s8 *key, s8 *src_value, void *object, s8 *fun, ub line);
#define dave_mongoc_upd_json(pCollection, key, src_value, object) __dave_mongoc_upd_json__(pCollection, key, src_value, object, (s8 *)__func__, (ub)__LINE__)

#endif

