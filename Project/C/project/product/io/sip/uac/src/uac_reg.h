/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_REG_H__
#define __UAC_REG_H__
#include "uac_class.h"

void uac_reg_init(void);
void uac_reg_exit(void);

void uac_reg(UACClass *pClass);

#endif

