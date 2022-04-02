/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RXTX_CONFIRM_TRANSFER_H__
#define __RXTX_CONFIRM_TRANSFER_H__
#include "base_macro.h"
#include "base_define.h"
#include "rxtx_param.h"

typedef dave_bool (* bin_ct_fun)(u8 dst_ip[4], u16 dst_port, s32 socket, CTNote *pNote);

void rxtx_confirm_transfer_init(bin_ct_fun ct_fun);

void rxtx_confirm_transfer_exit(void);

dave_bool rxtx_confirm_transfer_push(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data);

dave_bool rxtx_confirm_transfer_pop(s32 socket, ORDER_CODE order_id);

ub rxtx_confirm_transfer_out(s32 socket, dave_bool resend);

void rxtx_confirm_transfer_clean(s32 socket);

#endif

