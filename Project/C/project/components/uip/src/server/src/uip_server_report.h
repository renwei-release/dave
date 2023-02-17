/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_SERVER_REPORT_H__
#define __UIP_SERVER_REPORT_H__

void uip_server_report_init(void);

void uip_server_report_exit(void);

void uip_server_report(UIPStack *pRecvStack, UIPStack *pSendStack);

#endif

