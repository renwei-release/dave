/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_PARSING_H__
#define __UIP_PARSING_H__
#include "dave_base.h"
#include "uip_key_define.h"

UIPStack * uip_malloc(void);

void uip_free(UIPStack *pStack);

UIPStack * uip_clone(UIPStack *pStack);

UIPStack * uip_decode(ThreadId src, void *ptr, void *pJson);

void * uip_encode(UIPStack *pStack, dave_bool encode_body);

ub uip_encode_error(s8 *data_buf, ub data_length, RetCode ret);

#endif

