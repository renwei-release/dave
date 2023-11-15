/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_REDIS_H__
#define __DAVE_REDIS_H__

void * dave_redis_connect(s8 *ip, ub port);
void dave_redis_disconnect(void *context);

RetCode dave_redis_set(void *context, s8 *set_command);
ub dave_redis_get(void *context, s8 *get_command, u8 *bin_ptr, ub bin_len);
RetCode dave_redis_del(void *context, s8 *del_command);

void * dave_redis_command(void *context, s8 *command);

#endif

