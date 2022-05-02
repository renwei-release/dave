/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_SERVER_MONITOR_H__
#define __UIP_SERVER_MONITOR_H__
#include "dave_base.h"
#include "uip_key_define.h"

typedef void ( * uip_monitor_error)(ThreadId dst, RetCode ret, void *ptr, void *pJson);

void uip_server_monitor_init(uip_monitor_error error_fun);

void uip_server_monitor_exit(void);

void * uip_server_monitor_malloc(UIPStack *pStack);

UIPStack * uip_server_monitor_free(void *monitor_ptr);

void uip_server_monitor_time_consuming(ub time_consuming);

#endif

