/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RXTX_TOOLS_H__
#define __RXTX_TOOLS_H__
#include "base_macro.h"

ub rxtx_build_crc(u8 *package, ub package_len);

ub rxtx_build_crc_v2(MBUF *data, u8 *crc_ptr, ub crc_len);

ub rxtx_check_crc(u8 *package, ub package_len);

#endif

