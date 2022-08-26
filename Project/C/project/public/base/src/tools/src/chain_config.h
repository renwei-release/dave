/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __CHAIN_CONFIG_H__
#define __CHAIN_CONFIG_H__
#include "thread_chain.h"

void chain_config_reset(CFGUpdate *pUpdate);

dave_bool chain_config_type_enable(ChainType type);

#endif

