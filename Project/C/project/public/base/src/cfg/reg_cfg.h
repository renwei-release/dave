/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __REG_CFG_H__
#define __REG_CFG_H__

void reg_cfg_init(void);
void reg_cfg_exit(void);

dave_bool reg_cfg_reg(s8 *name, cfg_reg_fun reg_fun);

#endif

