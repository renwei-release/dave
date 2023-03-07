/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_AUTH_KEY_H__
#define __UIP_AUTH_KEY_H__
#include "dave_base.h"

s8 * uip_auth_key_build(s8 auth_key_str[DAVE_AUTH_KEY_STR_LEN], s8 *db_name, s8 *channel_name);

dave_bool uip_auth_key_check(s8 auth_key_str[DAVE_AUTH_KEY_STR_LEN], s8 *channel_name);

#endif

