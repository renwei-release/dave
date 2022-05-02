/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_TOOLS_H__
#define __UIP_TOOLS_H__
#include "dave_base.h"
#include "uip_key_define.h"

dave_bool uip_is_head(s8 *key);

void uip_write_stack(char *stack_name, UIPStack *pStack);

#endif

