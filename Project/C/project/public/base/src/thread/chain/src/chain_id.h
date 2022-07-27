/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __CHAIN_ID_H__
#define __CHAIN_ID_H__

void chain_id_reset(void);

s8 * chain_id(s8 *chain_id_ptr, ub chain_id_len);

ub chain_counter(void);

ub chain_msg_serial(void);

#endif

