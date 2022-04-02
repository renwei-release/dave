/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_HTTP_H__
#define __DAVE_HTTP_H__
#include "http_param.h"
#include "http_msg.h"

#define HTTP_THREAD_NAME "http"
#define DISTRIBUTOR_THREAD_NAME "distributor"

void dave_http_init(void);
void dave_http_exit(void);

#endif

