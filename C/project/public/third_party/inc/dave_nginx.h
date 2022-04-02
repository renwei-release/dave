/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_NGINX_H__
#define __DAVE_NGINX_H__
#include "dave_http.h"

void dave_nginx_init(void);

void dave_nginx_exit(void);

ErrCode dave_nginx_start(ub nginx_port, HTTPListenType type, ub cgi_port, s8 *nginx_path, s8 *pem_path, s8 *key_path);

ErrCode dave_nginx_stop(ub nginx_port);

#endif

