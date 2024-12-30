/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_MAIN_H__
#define __UAC_MAIN_H__
#include "uac_class.h"

void uac_main_init(void);
void uac_main_exit(void);

UACClass * uac_main_class(void);

#endif

