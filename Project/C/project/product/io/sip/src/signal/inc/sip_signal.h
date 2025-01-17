/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SIP_SIGNAL_H__
#define __SIP_SIGNAL_H__
#include "dave_base.h"
#include "t_lock.h"
#include "dave_osip.h"
#include "dave_rtp.h"

#define SIP_RECV_BUFFER_MAX 1024 * 4

typedef ub (*sip_recv_fun)(void *param, osip_message_t *pRecv);
typedef void (* call_start_fun)(void *call);
typedef void (* call_end_fun)(void *call);

typedef enum {
	SIPSignalType_server,
	SIPSignalType_client,
	SIPSignalType_max = 0x1fffffffffffffff
} SIPSignalType;

typedef struct {
	ub reg_timer_counter;
} SIPReg;

typedef struct {
	s8 call_data[128];

	osip_call_id_t *call_id;
	osip_from_t *from;
	osip_to_t *to;
	osip_cseq_t *cseq;
	RTP *rtp;

	call_start_fun start_fun;
	call_end_fun end_fun;

	dave_bool get_invite_request_intermediate_state;
	osip_message_t *invite_request;
	dave_bool get_bye_request_intermediate_state;
	osip_message_t *bye_request;

	void *signal;
	void *user_ptr;
} SIPCall;

typedef struct {
	SIPSignalType signal_type;
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
	s8 recv_buffer[SIP_RECV_BUFFER_MAX];

	sip_recv_fun reg_recv_fun;
	void *reg_recv_param;
	sip_recv_fun inv_recv_fun;
	void *inv_recv_param;
	sip_recv_fun bye_recv_fun;
	void *bye_recv_param;

	TLock request_pv;
	ub cseq_number;
	dave_bool get_register_request_intermediate_state;
	osip_message_t *register_request;

	SIPReg reg;
	void *call_id_kv;
	void *call_data_kv;
} SIPSignal;

void sip_signal_init(void);
void sip_signal_exit(void);

SIPSignal * sip_signal_creat(
	s8 *server_ip, s8 *server_port, s8 *username, s8 *password,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port);

void sip_signal_release(SIPSignal *pSignal);

void sip_signal_reg_reg(SIPSignal *pSignal, sip_recv_fun fun, void *param);
void sip_signal_reg_inv(SIPSignal *pSignal, sip_recv_fun fun, void *param);
void sip_signal_reg_bye(SIPSignal *pSignal, sip_recv_fun fun, void *param);

dave_bool sip_signal_recv(SocketRead *pRead);

void sip_signal_send(SIPSignal *pSignal, SIPCall *pCall, osip_message_t *sip);

#endif

