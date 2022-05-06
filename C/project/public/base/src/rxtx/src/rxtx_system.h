/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RXTX_SYSTEM_H__
#define __RXTX_SYSTEM_H__
#include "base_macro.h"
#include "dave_base.h"

dave_bool rxtx_system_rx(RXTX *pRxTx, ORDER_CODE order_id, ub frame_len, u8 *frame);

void rxtx_system_no_crc_tx(RXTX *pRxTx);

#endif

