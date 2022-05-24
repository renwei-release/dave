/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DOS_POP_H__
#define __DOS_POP_H__
#include "dave_base.h"

typedef RetCode (* pop_process_fun)(s8 *param, ub param_len);

void dos_pop_init(void);

void dos_pop_exit(void);

dave_bool dos_pop_analysis(s8 *param, ub param_len);

RetCode dos_pop_confirm(char *msg, pop_process_fun yes, pop_process_fun no);

RetCode dos_pop_input_request(s8 *msg, pop_process_fun process_fun);

#endif

