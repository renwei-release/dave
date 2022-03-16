/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#ifndef __DOS_TOOLS_H__
#define __DOS_TOOLS_H__
#include "dave_base.h"

ub dos_get_last_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len);

ub dos_get_one_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len);

#endif

