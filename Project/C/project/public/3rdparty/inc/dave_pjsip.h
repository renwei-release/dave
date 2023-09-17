/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_PJSIP_H__
#define __DAVE_PJSIP_H__
#include "dave_define.h"

typedef void (* pjsip_cb_fun)(void *ptr);

typedef enum {
	SIPTransportType_udp,
	SIPTransportType_tcp,
	SIPTransportType_tls,
	SIPTransportType_max = 0xffffffffffffffff
} SIPTransportType;

typedef enum {
	AudioCodec_G711 = 0,
	AudioCodec_G722,
	AudioCodec_G7221,
	AudioCodec_G729,
	AudioCodec_iLBC,
	AudioCodec_GSM,
	AudioCodec_H263,
	AudioCodec_H264,
	AudioCodec_max
} AudioCodec;

typedef enum {
	SIPStatus_NULL = 0,
	SIPStatus_CALLING,
	SIPStatus_INCOMING,
	SIPStatus_CONNECTING,
	SIPStatus_CONFIRMED,
	SIPStatus_DISCONNECTED,
	SIPStatus_Hold,
	SIPStatus_Online,
	SIPStatus_Offline,
	SIPStatus_CONF_NOTIFY,

	SIPStatus_100Trying = 100,
	SIPStatus_180Ringing = 180,
	SIPStatus_183SessionProgress = 183,
	SIPStatus_200OK = 200,
	SIPStatus_202Accept = 202,
	SIPStatus_400BadRequest = 400,
	SIPStatus_401Unauthorized = 401,
	SIPStatus_403Forbidden = 403,
	SIPStatus_408RequestTimeout = 408,
	SIPStatus_410Gone = 410,
	SIPStatus_420BadExtension = 420,
	SIPStatus_480TemporarilyUnavailable = 480,
	SIPStatus_481CallTSXDoesNotExist = 481,
	SIPStatus_487RequestTerminated = 487,
	SIPStatus_488NotAcceptableHere = 488,
	SIPStatus_490RequestUpdate = 490,
	SIPStatus_500ServerInternalError = 500,
	SIPStatus_503ServerUnavailable = 503,

	SIPStatus_max = 0xffffffffffffffff
} SIPStatus;

#define DAVE_ALLOWED_CODEC_MAX (3)

dave_bool dave_pjsip_init(ThreadId owner, s8 *server_addr, ub server_port, AudioCodec codec[DAVE_ALLOWED_CODEC_MAX]);

void dave_pjsip_exit(void);

void dave_pjsip_reg_call_cb(pjsip_cb_fun cb_fun);

void dave_pjsip_reg_incoming_cb(pjsip_cb_fun cb_fun);

void dave_pjsip_reg_media_state_cb(pjsip_cb_fun cb_fun);

void dave_pjsip_reg_incoming_message(pjsip_cb_fun cb_fun);

void dave_pjsip_reg_message_state(pjsip_cb_fun cb_fun);

sb dave_pjsip_login(s8 *server_addr, ub server_port, SIPTransportType transport_type,
	ub media_port, ub media_port_range,
	s8 *user, s8 *domain, s8 *pwd, s8 *impi,
	ub reg_timeout,
	pjsip_cb_fun register_fun);

dave_bool dave_pjsip_logout(sb acc_id);

sb dave_pjsip_make_call(sb acc_id, s8 *call_target);

dave_bool dave_pjsip_answer_call(sb call_id, SIPStatus code);

dave_bool dave_pjsip_hold_call(sb call_id);

dave_bool dave_pjsip_unhold_call(sb call_id);

ErrCode __dave_pjsip_hangup_call__(sb call_id, s8 *fun, ub line);
#define dave_pjsip_hangup_call(call_id) __dave_pjsip_hangup_call__(call_id, (s8 *)__func__, (ub)__LINE__)

dave_bool dave_pjsip_refer_call(sb call_id, s8 *refer_to);

dave_bool dave_pjsip_update_call(sb call_id);

dave_bool dave_pjsip_conf_subscribe(s8 *request_uri, pjsip_cb_fun state_fun);

dave_bool dave_pjsip_call_id_info(sb call_id, s8 *uri, s8 *id, s8 *to_tag, s8 *from_tag);

void dave_pjsip_register_option(dave_bool on);

dave_bool dave_pjsip_set_mute_mode(dave_bool mute, sb call_id);

dave_bool dave_pjsip_send_im(sb acc_id, s8 *target, s8 *mime_type, s8 *content);

#endif

