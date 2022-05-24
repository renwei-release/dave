/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_SERVER_REGISTER_H__
#define __UIP_SERVER_REGISTER_H__
#include "dave_base.h"

void uip_server_register_init(void);

void uip_server_register_exit(void);

RetCode uip_server_register(ThreadId src, s8 *method);

RetCode uip_server_unregister(s8 *method);

RetCode uip_server_register_data(s8 *thread, s8 *method);

ub uip_server_register_info(s8 *info_ptr, ub info_len);

#endif

