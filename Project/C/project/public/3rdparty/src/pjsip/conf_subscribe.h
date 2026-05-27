/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef __CONF_SUBSCRIBE_H__
#define __CONF_SUBSCRIBE_H__

dave_bool conf_subscribe_init(ThreadId owner, sb acc_id);

void conf_subscribe_exit(void);

dave_bool conf_subscribe(s8 *request_uri, pjsip_cb_fun state_fun);

#endif

