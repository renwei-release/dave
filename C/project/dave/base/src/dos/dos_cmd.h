/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DOS_CMD_H__
#define __DOS_CMD_H__
#include "dave_base.h"

typedef ErrCode (* cmd_process_fun)(s8 *param, ub param_len);
typedef ErrCode (* help_process_fun)(void);

void dos_cmd_init(void);

void dos_cmd_exit(void);

void dos_cmd_reset(void);

void dos_cmd_analysis(s8 *input_ptr, ub input_len);

void dos_help_analysis(s8 *cmd_ptr, ub cmd_len);

ErrCode dos_cmd_register(char *cmd, cmd_process_fun fun, help_process_fun help_fun);

ErrCode dos_cmd_talk_register(s8 *cmd, cmd_process_fun fun, help_process_fun help_fun);

MBUF * dos_cmd_list(void);

void dos_cmd_list_show(void);

#endif

