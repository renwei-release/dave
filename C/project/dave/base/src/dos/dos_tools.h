/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DOS_TOOLS_H__
#define __DOS_TOOLS_H__
#include "dave_base.h"

ub dos_get_last_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len);
ub dos_get_one_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len);

ub dos_get_bool(s8 *cmd_ptr, ub cmd_len, dave_bool *bool_value);
ub dos_get_str(s8 *cmd_ptr, ub cmd_len, s8 *str_ptr, ub str_len);

#endif

