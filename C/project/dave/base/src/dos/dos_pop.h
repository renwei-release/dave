/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#ifndef __DOS_POP_H__
#define __DOS_POP_H__
#include "dave_base.h"

typedef ErrCode (* pop_process_fun)(s8 *param, ub param_len);

void dos_pop_init(void);

void dos_pop_exit(void);

dave_bool dos_pop_analysis(s8 *param, ub param_len);

ErrCode dos_pop_confirm(char *msg, pop_process_fun yes, pop_process_fun no);

ErrCode dos_pop_input_request(s8 *msg, pop_process_fun process_fun);

#endif

