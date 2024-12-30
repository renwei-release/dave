/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_CLASS_H__
#define __UAC_CLASS_H__
#include "dave_osip.h"
#include "uac_signal.h"
#include "uac_rtp.h"

void uac_class_init(void);
void uac_class_exit(void);

typedef struct {
	s8 phone_number[128];
	osip_call_id_t *call_id;
	osip_from_t *from;
	osip_to_t *to;
	osip_cseq_t *cseq;

	UACRTP *rtp;
} UACCall;

typedef struct {
	UACCall call;

	UACSignal signal;
} UACClass;

UACClass * uac_class_creat(
	s8 *server_ip, s8 *server_port, s8 *username, s8 *password,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port);

void uac_class_release(UACClass *pClass);

void uac_class_reg_reg(void *pClass, uac_recv_fun fun);
void uac_class_reg_inv(void *pClass, uac_recv_fun fun);
void uac_class_reg_bye(void *pClass, uac_recv_fun fun);

void uac_class_recv(SocketRead *pRead);

void uac_class_send(UACClass *pClass, osip_message_t *sip);

#endif

