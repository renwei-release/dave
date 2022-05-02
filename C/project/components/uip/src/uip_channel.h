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

RetCode uip_channel_verify(s8 *channel, s8 *auth_key_str);

s8 * uip_channel_inq(s8 *channel);

#endif

