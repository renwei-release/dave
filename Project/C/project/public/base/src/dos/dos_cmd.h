/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DOS_CMD_H__
#define __DOS_CMD_H__
#include "dave_base.h"

void dos_cmd_init(void);

void dos_cmd_exit(void);

void dos_cmd_analysis(s8 *input_ptr, ub input_len);

void dos_help_analysis(s8 *cmd_ptr, ub cmd_len);

RetCode dos_cmd_talk_reg(s8 *cmd, dos_cmd_fun cmd_fun, dos_help_fun help_fun);

MBUF * dos_cmd_list(void);

void dos_cmd_list_show(void);

#endif

