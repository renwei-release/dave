/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_AUTOMATIC_H__
#define __UAC_AUTOMATIC_H__

void uac_automatic_init(void);
void uac_automatic_exit(void);

dave_bool uac_automatic_send(UACClass *pClass, osip_message_t *sip);

void uac_automatic_recv(UACClass *pClass, osip_message_t *sip, int status_code);

#endif

