/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_SIGNAL_H__
#define __UAC_SIGNAL_H__
#include "dave_osip.h"

#define UAC_CLASS_RECV_BUFFER_MAX 1024 * 16

typedef ub (*uac_recv_fun)(void *pClass, osip_message_t *pRecv);

typedef struct {
	s32 signal_socket;

	s8 server_ip[64];
	s8 server_port[16];
	s8 username[64];
	s8 password[16];
	s8 local_ip[64];
	s8 local_port[16];
	s8 rtp_ip[16];
	s8 rtp_port[16];

	ub recv_r_index;
	ub recv_e_index;
	ub recv_w_index;
	s8 recv_buffer[UAC_CLASS_RECV_BUFFER_MAX];

	uac_recv_fun reg_recv_fun;
	uac_recv_fun inv_recv_fun;
	uac_recv_fun bye_recv_fun;

	TLock request_pv;
	ub cseq_number;
	dave_bool get_register_request_intermediate_state;
	osip_message_t *register_request;
	dave_bool get_invite_request_intermediate_state;
	osip_message_t *invite_request;
	dave_bool get_bye_request_intermediate_state;
	osip_message_t *bye_request;

	ub reg_timer_counter;
} UACSignal;

#endif

