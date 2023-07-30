/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DOS_TOOLS_H__
#define __DOS_TOOLS_H__
#include "dave_base.h"

#ifdef __cplusplus
extern "C"{
#endif

ub dos_get_last_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len);
ub dos_get_one_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len);
s8 * dos_get_user_input(s8 *give_user_msg, ub wait_second);

ub dos_load_bool(s8 *cmd_ptr, ub cmd_len, dave_bool *bool_value);
ub dos_load_ub(s8 *cmd_ptr, ub cmd_len, ub *ub_data);
ub dos_load_string(s8 *cmd_ptr, ub cmd_len, s8 *str_ptr, ub str_len);

RetCode dos_opt_u16_cfg(char *msg, s8 *cmd_ptr, ub cmd_len, s8 *u16_name);
RetCode dos_opt_ip_cfg(char *msg, s8 *cmd_ptr, ub cmd_len, s8 *server_name);
RetCode dos_opt_ip_port_cfg(char *msg, s8 *cmd_ptr, ub cmd_len, s8 *server_name, s8 *port_name);

#ifdef __cplusplus
} //extern "C"
#endif

#endif

