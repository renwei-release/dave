/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 *
 * ############################# IMPORTANT INFORMATION ############################
 * The code of this file is automatically generated by tools(Tools/rpc),
 * please do not modify it manually!
 * ############################# IMPORTANT INFORMATION ############################
 * ================================================================================
 */

#ifndef _T_RPC_RPCDATA_H__
#define _T_RPC_RPCDATA_H__
#include "dave_base.h"

void * t_rpc_ver3_zip(ub msg_id, void *msg_body, ub msg_len);
dave_bool t_rpc_ver3_unzip(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len);

#endif
