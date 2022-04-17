/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __NGINX_CONF_H__
#define __NGINX_CONF_H__

dave_bool nginx_conf_add(ub work_process, ub nginx_port, HTTPListenType type, ub cgi_port, s8 *nginx_path, s8 *pem_path, s8 *key_path);

ub nginx_conf_del(ub work_process, ub nginx_port);

ub nginx_conf_number(void);

#endif

