/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_CHANNEL_H__
#define __UIP_CHANNEL_H__
#include "dave_base.h"
#include "uip_parsing.h"

void uip_channel_init(void);

void uip_channel_exit(void);

void uip_channel_reset(void);

RetCode uip_channel_verify(s8 *channel_name, s8 *auth_key, s8 *allow_method);

s8 * uip_channel_inq(s8 *channel_name);

s8 * uip_channel_add(s8 *channel_name, s8 *user_input_auth_key);

dave_bool uip_channel_del(s8 *channel_name);

dave_bool uip_channel_add_method(s8 *channel_name, s8 *allow_method);

ub uip_channel_info(s8 *info_ptr, ub info_len);

#endif

