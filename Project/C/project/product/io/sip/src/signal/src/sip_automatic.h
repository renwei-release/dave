/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SIP_AUTOMATIC_H__
#define __SIP_AUTOMATIC_H__

dave_bool sip_automatic_send(SIPSignal *pSignal, osip_message_t *sip);

void sip_automatic_recv(SIPSignal *pSignal, osip_message_t *sip, int status_code);

#endif

