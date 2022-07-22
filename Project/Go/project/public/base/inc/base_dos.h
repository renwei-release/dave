/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_DOS_H__
#define __BASE_DOS_H__
#include "dave_base.h"

#define DOS_THREAD_NAME "dos"

void base_dos_init(void);
void base_dos_exit(void);

typedef RetCode (* dos_cmd_fun)(s8 *param_ptr, ub param_len);
typedef RetCode (* dos_help_fun)(void);
RetCode dos_cmd_reg(const char *cmd, dos_cmd_fun cmd_fun, dos_help_fun help_fun);
void dos_print(const char *fmt, ...);

#endif

