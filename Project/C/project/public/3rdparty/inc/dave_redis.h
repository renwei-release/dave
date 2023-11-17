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

void * dave_redis_command(void *context, s8 *command);

#endif

