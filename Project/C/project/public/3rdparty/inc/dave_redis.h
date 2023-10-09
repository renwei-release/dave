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

/* Synchronous API */
void * dave_redis_connect(s8 *ip, ub port);

void dave_redis_free_reply(void *reply);

void dave_redis_disconnect(void *context);

/* Redis command which returns integer value */
ErrCode dave_redis_int_command(void * context, s8 * command, sb *ret_value);

#endif

