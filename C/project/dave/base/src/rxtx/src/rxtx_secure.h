/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RXTX_SECURE_H__
#define __RXTX_SECURE_H__
#include "base_macro.h"
#include "base_define.h"

MBUF * rxtx_simple_encode_request(MBUF *data);

void rxtx_simple_encode_release(MBUF *encode_package);

u8 * rxtx_simple_decode_request(u8 *decode_package, ub decode_package_len, ub *package_len);

void rxtx_simple_decode_release(u8 *package);

#endif

