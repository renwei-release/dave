/*
 * ================================================================================
 * (c) Copyright 2018 Kevin Chen(CMI) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.10.12.
 * ================================================================================
 */

#ifndef __DAVE_REDIS_H__
#define __DAVE_REDIS_H__
#include "dave_define.h"

void * dave_redis_connect(s8 *ip, ub port);
void dave_redis_disconnect(void *context);

RetCode dave_redis_set(void *context, s8 *set_command);
ub dave_redis_get(void *context, s8 *get_command, u8 *bin_ptr, ub bin_len);
RetCode dave_redis_del(void *context, s8 *del_command);

#endif

