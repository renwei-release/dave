/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_CALL_H__
#define __UAC_CALL_H__

void uac_call_init(void);
void uac_call_exit(void);

void uac_bye(void);

void uac_call(s8 *phone_number);

#endif

