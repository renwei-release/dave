/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __NGINX_ACTION_H__
#define __NGINX_ACTION_H__

void nginx_action_init(void);

void nginx_action_exit(void);

dave_bool nginx_action_conf_add(ub work_process, ub nginx_port, HTTPListenType type, ub cgi_port, s8 *nginx_path, s8 *pem_path, s8 *key_path);

ub nginx_action_conf_del(ub work_process, ub nginx_port);

ErrCode nginx_action_start(void);

void nginx_action_stop(void);

#endif

