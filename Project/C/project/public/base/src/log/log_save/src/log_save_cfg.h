/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_SAVE_CFG_H__
#define __LOG_SAVE_CFG_H__

void log_save_cfg_init(void);

void log_save_cfg_exit(void);

dave_bool log_save_type_enable(ChainType type);

dave_bool log_save_json_enable(void);

dave_bool log_save_txt_enable(void);

ub log_save_days(void);

#endif

