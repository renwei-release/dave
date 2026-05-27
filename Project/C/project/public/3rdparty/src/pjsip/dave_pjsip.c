/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(PJSIP_3RDPARTY)
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h> 
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "party_log.h"
#include "pjsua.h"
#include "pjsip.h"
#include <pjsua-lib/pjsua_internal.h>
#include <pjmedia/transport.h>
#include "pjsip_endpoint.h"
#include "conf_subscribe.h"

extern pj_bool_t pjmedia_add_bandwidth_tias_in_sdp;
extern pj_status_t pjsua_start_mwi(pjsua_acc_id acc_id, pj_bool_t force_renew);

static ThreadId _owner_thread_id = INVALID_THREAD_ID;
static pj_thread_desc _pjsip_thread_desc;
static pj_thread_t *_pjsip_thread;
struct pjsip_module *_mod_xfer = NULL;

static pjsip_regc *_regc = NULL;
static pj_timer_entry _regc_timer;
static pjsua_transport_id _transport_id = -1;

static SIPNotifyInfo _register_info;
static pjsip_cb_fun _register_state_fun = NULL;
static SIPNotifyInfo _call_info;
static pjsip_cb_fun _call_state_fun = NULL;
static SIPNotifyInfo _incoming_info;
static pjsip_cb_fun _incoming_state_fun = NULL;
static pjsip_cb_fun _media_state_fun = NULL;
static pjsip_cb_fun _incoming_message_fun = NULL;
static pjsip_cb_fun _message_state_fun = NULL;

static void
_pjsip_notify_owner(pjsip_cb_fun fun, SIPNotifyInfo *pInfo)
{

}

static void
_pjsip_stop_register_timer(void)
{
	if(_regc != NULL)
	{
		if(_regc->timer.id != 0)
		{
			_regc_timer = _regc->timer;

			pjsip_endpt_cancel_timer(_regc->endpt, &(_regc->timer));
			_regc->timer.id = 0;
		}
	}
}

static void
_pjsip_start_register_timer(void)
{
	pj_time_val delay = { 0, 0};

	if(_regc != NULL)
	{
		if((_regc->timer.id == 0) && (_regc_timer.cb != NULL))
		{
			_regc->timer.cb = _regc_timer.cb;
			_regc->timer.id = 1;
			_regc->timer.user_data = _regc;

			delay.sec = _regc->expires - _regc->delay_before_refresh;

			pjsip_endpt_schedule_timer(_regc->endpt, &(_regc->timer), &delay);
		}
	}
}

static void
_pjsip_call_info_notify(SIPMethod method, SIPStatus state, pjsua_acc_id acc_id, pjsua_call_id call_id, char *contact)
{
	_call_info.method = method;
	_call_info.state = state;
	if(_call_info.state != SIPStatus_NULL)
	{
		_call_info.acc_id = (sb)(acc_id);
		_call_info.call_id = (sb)(call_id);
		if(contact != NULL)
		{
			ub copy_len;

			copy_len = dave_strcpy(_call_info.uri, (s8 *)(&(contact[5])), DAVE_URL_LEN);
			if(_call_info.uri[copy_len - 1] == '>')
			{
				_call_info.uri[copy_len - 1] = '\0';
			}
		}
		else
		{
			_call_info.uri[0] = '\0';
		}

		_pjsip_notify_owner(_call_state_fun, &_call_info);
	}
}

static void
_pjsip_incoming_info_notify(SIPMethod method, SIPStatus state, pjsua_acc_id acc_id, pjsua_call_id call_id, s8 *user, ub user_len)
{
	_incoming_info.method = method;
	_incoming_info.state = state;
	_incoming_info.acc_id = acc_id;
	_incoming_info.call_id = call_id;
	dave_strcpy(_incoming_info.uri, user, user_len);

	PARTYDEBUG("Incoming Call ID:%d!", call_id);

	_pjsip_notify_owner(_incoming_state_fun, &_incoming_info);
}

/* Callback called by the library upon receiving incoming call */
static void
_pjsip_on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata)
{
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);

    PARTYDEBUG("Incoming Call ID:%d from:%s!", call_id, ci.remote_info.ptr);

	_pjsip_incoming_info_notify(SIPMethod_INVITE, SIPStatus_INCOMING, ci.acc_id, call_id, (s8 *)(ci.remote_info.ptr), (ub)(ci.remote_info.slen));
}

static void
_pjsip_on_call_tsx_state(pjsua_call_id call_id, pjsip_transaction *tsx, pjsip_event *e)
{
	static pjsua_call_id up_call_id = -1;
	static SIPMethod up_method = SIPMethod_max;
	static int up_status = 0;
	static pj_int32_t up_cseq = 0;
	SIPMethod method = SIPMethod_max;
    const pjsip_method refer_method = {
		PJSIP_OTHER_METHOD,
		{ "REFER", 5 }
    };
    const pjsip_method update_method = {
		PJSIP_OTHER_METHOD,
		{ "UPDATE", 6 }
    };
    const pjsip_method prack_method = {
		PJSIP_OTHER_METHOD,
		{ "PRACK", 5 }
    };
    const pjsip_method notify_method = {
		PJSIP_OTHER_METHOD,
		{ "NOTIFY", 6 }
    };

	PARTYDEBUG("Call ID:%d method:%d state:%d type:%d",
		call_id, tsx->method.id, tsx->status_code, e->type);

	if(tsx->method.id != PJSIP_OTHER_METHOD)
	{
		method = (SIPMethod)(tsx->method.id);
	}
	else
	{
		if(pjsip_method_cmp(&(tsx->method), &refer_method) == 0)
		{
			method = SIPMethod_REFER;
		}
		else if(pjsip_method_cmp(&(tsx->method), &update_method) == 0)
		{
			method = SIPMethod_UPDATE;
		}
		else if(pjsip_method_cmp(&(tsx->method), &prack_method) == 0)
		{
			method = SIPMethod_PRACK;
		}
		else if(pjsip_method_cmp(&(tsx->method), &notify_method) == 0)
		{
			method = SIPMethod_NOTIFY;
		}
	}

	if(method < SIPMethod_max)
	{
		if((up_call_id != call_id) || (up_method != method) || (up_status != tsx->status_code) || (up_cseq != tsx->cseq))
		{
			up_call_id = call_id;
			up_method = method;
			up_status = tsx->status_code;
			up_cseq = tsx->cseq;

			_pjsip_call_info_notify(method, (SIPStatus)(tsx->status_code), 0, call_id, NULL);
		}
	}
	else
	{
		PARTYABNOR("What is this method:%s?", tsx->method.name.ptr);
	}
}

/* Callback called by the library when call's media state has changed */
static void
_pjsip_on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);
    
    PARTYDEBUG("Call media state change %d state=%s", call_id, ci.state_text.ptr);

    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE)
    {
        // When media is active, connect call to sound device.
        PARTYDEBUG("call media status acive");
        pjsua_conf_connect(ci.conf_slot, 0);
        pjsua_conf_connect(0, ci.conf_slot);
    }

	if(_media_state_fun != NULL)
	{
		_media_state_fun(NULL);
	}
}

static void
_pjsip_on_call_transfer_request(pjsua_call_id call_id, const pj_str_t *dst, pjsip_status_code *code)
{
	*code = PJSIP_SC_ACCEPTED;

	PARTYDEBUG("Call ID:%d %s<%d>", call_id, dst->ptr, *code);
}

static void
_pjsip_on_call_transfer_request2(pjsua_call_id call_id, const pj_str_t *dst, pjsip_status_code *code, pjsua_call_setting *opt)
{
	*code = PJSIP_SC_ACCEPTED;

	PARTYDEBUG("Call ID:%d %s<%d>", call_id, dst->ptr, *code);
}

static void
_pjsip_on_reg_state(pjsua_acc_id acc_id)
{
	PARTYDEBUG("");
}

static void
_pjsip_on_reg_info(pjsua_acc_id acc_id, pjsua_reg_info *info)
{
	PARTYDEBUG("");

	_regc = info->regc;

	if(info->cbparam->code != PJSIP_SC_OK)
	{
		PARTYDEBUG("status:%d code:%d", info->cbparam->status, info->cbparam->code);
	}

	if((info->cbparam->status == PJ_SUCCESS) && (info->cbparam->code == PJSIP_SC_OK))
		_register_info.state = SIPStatus_Online;
	else
		_register_info.state = SIPStatus_Offline;
	_register_info.acc_id = acc_id;
	_register_info.uri[0] = '\0';

	if((info != NULL) && (info->cbparam != NULL) && (info->cbparam->rdata != NULL) && (info->cbparam->rdata->msg_info.via != NULL))
	{
		if(info->cbparam->rdata->msg_info.via->recvd_param.ptr != NULL)
		{
			dave_strcpy(_register_info.uri,
				info->cbparam->rdata->msg_info.via->recvd_param.ptr,
				info->cbparam->rdata->msg_info.via->recvd_param.slen);
		}
	}

	_pjsip_notify_owner(_register_state_fun, &_register_info);
}

static void
_pjsip_on_incoming_subscribe(pjsua_acc_id acc_id, pjsua_srv_pres *srv_pres, pjsua_buddy_id buddy_id, const pj_str_t *from, pjsip_rx_data *rdata, pjsip_status_code *code, pj_str_t *reason, pjsua_msg_data *msg_data)
{
	PARTYDEBUG("acc_id:%d from:%s", acc_id, from->ptr);
}

static void
_pjsip_on_srv_subscribe_state(pjsua_acc_id acc_id, pjsua_srv_pres *srv_pres, const pj_str_t *remote_uri, pjsip_evsub_state state, pjsip_event *event)
{
	PARTYDEBUG("acc_id:%d remote_uri:%s", acc_id, remote_uri->ptr);
}

static void
_pjsip_on_buddy_state(pjsua_buddy_id buddy_id)
{
	PARTYDEBUG("buddy_id:%d", buddy_id);
}

static void
_pjsip_on_buddy_evsub_state(pjsua_buddy_id buddy_id, pjsip_evsub *sub, pjsip_event *event)
{
	PARTYDEBUG("buddy_id:%d", buddy_id);
}

static void
_pjsip_on_nat_detect(const pj_stun_nat_detect_result *res)
{
    if (res->status != PJ_SUCCESS) {
        PARTYDEBUG("NAT detection failed state %s",res->status);
    }else {
        PARTYDEBUG("NAT detected as %s",res->nat_type_name);
    }
}

static pj_status_t
_pjsip_creat_transport_(int type, pjsua_transport_id *transport_id, unsigned port)
{
	pj_status_t status;
	pjsua_transport_config transport_cfg;
	pjsua_acc_id acc_id;

	pjsua_transport_config_default(&transport_cfg);
    if(port != 0)
    {
        transport_cfg.port = port;
        transport_cfg.port_range = 1000;
    }
	status = pjsua_transport_create(type, &transport_cfg, transport_id);
	if((status != PJ_SUCCESS) && (status != PJSIP_ETYPEEXISTS))
	{
		PARTYABNOR("transport:%d create failed:%d", type, status);
		return status;
	}

	pjsua_acc_add_local(*transport_id, PJ_TRUE, &acc_id);
	pjsua_acc_set_online_status(acc_id, PJ_TRUE);

	return status;
}

static pjsua_transport_id
_pjsip_creat_transport(SIPTransportType transport_type)
{
	pj_status_t status;
	int type;
	pjsua_transport_id transport_id = -1;

	if(transport_type == SIPTransportType_udp)
	{
		type = PJSIP_TRANSPORT_UDP;
	}
	else if(transport_type == SIPTransportType_tcp)
	{
		type = PJSIP_TRANSPORT_TCP;
	}
	else
	{
		type = PJSIP_TRANSPORT_TLS;
	}

	status = _pjsip_creat_transport_(type, &transport_id, 0);
	if((status != PJ_SUCCESS) && (status != PJSIP_ETYPEEXISTS))
	{
		PARTYDEBUG("transport %d create failed:%d", type, status);
		transport_id = -1;
	}

	return transport_id;
}

static void
_pjsip_show_codec_info(void)
{
    pjmedia_codec_mgr *mgr = pjmedia_endpt_get_codec_mgr(pjsua_var.med_endpt);
    unsigned index;

    for(index=0; index<mgr->codec_cnt; ++index)
    {
        PARTYLOG("index:%d id:%s", index, mgr->codec_desc[index].id);
    }

	PARTYLOG("");
}

static void
_pjsip_remove_codec(char *codec)
{
    pjmedia_codec_mgr *mgr = pjmedia_endpt_get_codec_mgr(pjsua_var.med_endpt);
    unsigned index, remove;

	if(mgr == NULL)
		return;

    for(index=0; index<mgr->codec_cnt; index++)
    {
        if(dave_strcmp((s8 *)(mgr->codec_desc[index].id), (s8 *)codec) == dave_true)
        {
            for(remove=index; (remove+1)<mgr->codec_cnt; remove++)
            {
                mgr->codec_desc[remove] = mgr->codec_desc[remove + 1];
            }
            if(mgr->codec_cnt > 0)
            {
                mgr->codec_cnt --;
            }
        }
    }
}

static dave_bool
_pjsip_has_the_codec(AudioCodec codec[DAVE_ALLOWED_CODEC_MAX], AudioCodec detected_codec)
{
	ub codec_index;

	for(codec_index=0; codec_index<DAVE_ALLOWED_CODEC_MAX; codec_index++)
	{
		if(codec[codec_index] == detected_codec)
		{
			return dave_true;
		}
	}

	return dave_false;
}

static pj_status_t
_pjsip_set_codec_priority(char *codec, pjmedia_codec_priority priority)
{
    pjmedia_codec_mgr *mgr = pjmedia_endpt_get_codec_mgr(pjsua_var.med_endpt);
    pj_str_t codec_id = {NULL, 0};
    pj_status_t status;

    codec_id = pj_str(codec);
    status = pjmedia_codec_mgr_set_codec_priority(mgr, &codec_id, priority);
    if(status != PJ_SUCCESS)
    {
		PARTYLOG("set codec:%s priority failed:%d", codec, status);
		_pjsip_show_codec_info();
    }

    return status;
}

static void
_pjsip_update_codec_priority(AudioCodec codec[DAVE_ALLOWED_CODEC_MAX])
{
	dave_bool del_g711 = dave_false;

	_pjsip_remove_codec("speex/16000/1");
	_pjsip_remove_codec("speex/8000/1");
	_pjsip_remove_codec("speex/32000/1");
	_pjsip_remove_codec("L16/44100/1");
	_pjsip_remove_codec("L16/44100/2");

	if(_pjsip_has_the_codec(codec, AudioCodec_G711) == dave_true)
	{
		_pjsip_set_codec_priority("PCMA/8000/1", PJMEDIA_CODEC_PRIO_HIGHEST);
	}

	if(_pjsip_has_the_codec(codec, AudioCodec_GSM) == dave_true)
	{
		_pjsip_set_codec_priority("GSM/8000/1", PJMEDIA_CODEC_PRIO_HIGHEST);

		del_g711 = dave_true;
	}

	if(_pjsip_has_the_codec(codec, AudioCodec_iLBC) == dave_true)
	{
		_pjsip_set_codec_priority("iLBC/8000/1", PJMEDIA_CODEC_PRIO_HIGHEST);

		del_g711 = dave_true;
	}

	if(del_g711 == dave_true)
	{
		_pjsip_remove_codec("PCMA/8000/1");
		_pjsip_remove_codec("PCMU/8000/1");
	}
}

static pjsip_module *
_pjsip_get_module(char *module_name)
{
	pjsip_endpoint *endpt = pjsua_var.endpt;
	ub modules_index;

	for(modules_index=0; modules_index<PJSIP_MAX_MODULE; modules_index++)
	{
		if(endpt->modules[modules_index] != NULL)
		{
			if(dave_strcmp((s8 *)(endpt->modules[modules_index]->name.ptr), (s8 *)module_name) == dave_true)
			{
				return endpt->modules[modules_index];
			}
		}
	}

	PARTYABNOR("can't find the module:%s", module_name);

	return NULL;
}

/** Callback called by the library upon receiving instant message */
static void
_pjsip_on_pager(pjsua_call_id call_id, const pj_str_t *from,
                const pj_str_t *to, const pj_str_t *contact,
                const pj_str_t *mime_type, const pj_str_t *body)
{
	SIPMessageInfo *info;
	s8 mime_type_str[128];
	s8 temp[DAVE_URL_LEN];
	int body_len = (body->slen > (DAVE_IM_LENGTH - 1)) ? (DAVE_IM_LENGTH - 1) : (int)(body->slen);

	info = dave_ralloc(sizeof(SIPMessageInfo));

	dave_strcpy(mime_type_str, mime_type->ptr, mime_type->slen + 1);
    dave_memcpy(temp, from->ptr, from->slen);
	copy_the_url_number((s8 *)(info->from), temp);
    dave_memcpy(temp, to->ptr, to->slen);
	copy_the_url_number((s8 *)(info->to), temp);
    dave_memcpy(info->body, body->ptr, body_len);

	PARTYLOG("mime:%s %s->%s body:%s",
		mime_type_str,
		info->from, info->to,
		info->body);

	if(dave_strcmp(mime_type_str, "text/plain") == dave_true)
	{
		if(_incoming_message_fun != NULL)
		{
			_incoming_message_fun(info);
		}
	}

	dave_free(info);
}

static void
_pjsip_on_pager_status(pjsua_call_id call_id,
                       const pj_str_t *to,
                       const pj_str_t *body,
                       void *user_data,
                       pjsip_status_code status,
                       const pj_str_t *reason)
{
    SIPMessageStatus *msgStatus = dave_ralloc(sizeof(SIPMessageStatus));

    msgStatus->state = (int)status;
    dave_memcpy(msgStatus->to, to->ptr, to->slen);
    dave_memcpy(msgStatus->body, body->ptr, body->slen);

	if(_message_state_fun != NULL)
	{
		_message_state_fun(msgStatus);
	}

	dave_free(msgStatus);
}

static void
_pjsip_thread_register(void)
{
	static dave_bool register_flag = dave_false;
	pj_status_t status;

    if((register_flag == dave_false) && (!pj_thread_is_registered()))
    {
    	register_flag = dave_true;

        pj_bzero(_pjsip_thread_desc, sizeof(pj_thread_desc));

        status = pj_thread_register("davepjsip", _pjsip_thread_desc, &_pjsip_thread);
        if(status != PJ_SUCCESS)
        {
            PARTYABNOR("pjsua thread register failed:%d", status);
        }
    }
}

static dave_bool
_pjsip_booting(s8 *server_addr, ub server_port, AudioCodec codec[DAVE_ALLOWED_CODEC_MAX])
{
	pj_status_t status;
	pjsua_config sua_cfg;
	pjsua_logging_config log_cfg;
	pjsua_media_config media_cfg;
	char outbound_proxy[256];

	pj_log_set_level(0);

    /* Create pjsua first! */
    status = pjsua_create();
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("pjsua create failed:%d", status);
        return dave_false;
	}

    pjmedia_add_bandwidth_tias_in_sdp = PJ_TRUE;

	dave_snprintf((s8 *)outbound_proxy, sizeof(outbound_proxy), "<sip:%s:%d;lr>", server_addr, server_port);

    /* Init pjsua */
    pjsua_config_default(&sua_cfg);
	sua_cfg.max_calls = 32;
	sua_cfg.outbound_proxy_cnt = 1;
	sua_cfg.outbound_proxy[0] = pj_str(outbound_proxy);
	sua_cfg.nat_type_in_sdp = 0;
	sua_cfg.require_100rel = PJSUA_100REL_MANDATORY;
	sua_cfg.use_timer = PJSUA_SIP_TIMER_REQUIRED;
	sua_cfg.user_agent = pj_str((char *)dave_verno());

	sua_cfg.cb.on_call_state = NULL;
    sua_cfg.cb.on_incoming_call = _pjsip_on_incoming_call;
	sua_cfg.cb.on_call_tsx_state = _pjsip_on_call_tsx_state;
    sua_cfg.cb.on_call_media_state = _pjsip_on_call_media_state;
	sua_cfg.cb.on_call_transfer_request = _pjsip_on_call_transfer_request;
	sua_cfg.cb.on_call_transfer_request2 = _pjsip_on_call_transfer_request2;
	sua_cfg.cb.on_call_transfer_status = NULL;
	sua_cfg.cb.on_transport_state = NULL;
    sua_cfg.cb.on_reg_state = _pjsip_on_reg_state;
	sua_cfg.cb.on_reg_state2 = _pjsip_on_reg_info;
	sua_cfg.cb.on_incoming_subscribe = _pjsip_on_incoming_subscribe;
	sua_cfg.cb.on_srv_subscribe_state = _pjsip_on_srv_subscribe_state;
	sua_cfg.cb.on_buddy_state = _pjsip_on_buddy_state;
	sua_cfg.cb.on_buddy_evsub_state = _pjsip_on_buddy_evsub_state;
    sua_cfg.cb.on_nat_detect = _pjsip_on_nat_detect;
    sua_cfg.cb.on_pager = _pjsip_on_pager;
    sua_cfg.cb.on_pager_status = _pjsip_on_pager_status;
    
    pjsua_logging_config_default(&log_cfg);
	log_cfg.console_level = 1; // 4;
    log_cfg.level = 5; // 5

    pjsua_media_config_default(&media_cfg);
    media_cfg.clock_rate = 16000;
    media_cfg.snd_clock_rate = 16000;
    media_cfg.audio_frame_ptime = PJSUA_DEFAULT_AUDIO_FRAME_PTIME;
    media_cfg.max_media_ports = PJSUA_MAX_CONF_PORTS;
    media_cfg.ilbc_mode = PJSUA_DEFAULT_ILBC_MODE;
    media_cfg.ptime = 1;
    media_cfg.no_vad = PJ_FALSE;

    status = pjsua_init(&sua_cfg, &log_cfg, &media_cfg);
    if(status != PJ_SUCCESS)
    {
        PARTYABNOR("pjsua init failed:%d", status);
        return dave_false;
    }

	_pjsip_thread_register();

    _pjsip_update_codec_priority(codec);

	_mod_xfer = _pjsip_get_module("mod-refer");

	/* Initialization is done, now start pjsua */
	status = pjsua_start();
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("start pjsua failed:%d!", status);
		return dave_false;
	}

	return dave_true;
}

static void
_pjsip_hangup_all(void)
{
	if(_owner_thread_id != INVALID_THREAD_ID)
	{
		pjsua_call_hangup_all();
	}
}

static sb
_pjsip_account_login(
	s8 *server_addr, ub server_port, SIPTransportType transport_type,
	ub media_port, ub media_port_range,
	s8 *user, s8 *domain, s8 *pwd, s8 *impi,
	ub reg_timeout)
{
	pj_status_t status;
    pjsua_acc_config acc_cfg;
    pjsua_acc_id acc_id;
    char account_str[256], uri_str[256], impi_str[256];
    
    pjsua_acc_config_default(&acc_cfg);

    dave_snprintf((s8 *)account_str, sizeof(account_str), "sip:%s@%s", user, domain);
    acc_cfg.id = pj_str(account_str);
	dave_snprintf((s8 *)uri_str, sizeof(uri_str), "sip:%s:%d", server_addr, server_port);
	acc_cfg.reg_uri = pj_str(uri_str);
	acc_cfg.require_100rel = PJSUA_100REL_NOT_USED;
	acc_cfg.use_timer = PJSUA_SIP_TIMER_ALWAYS;
	acc_cfg.timer_setting.min_se = 90;
	acc_cfg.timer_setting.sess_expires = 3600;
	acc_cfg.use_rfc5626 = PJ_TRUE;
	acc_cfg.reg_timeout = (unsigned)reg_timeout;
	acc_cfg.reg_delay_before_refresh = 8;
	acc_cfg.mwi_enabled = PJ_FALSE;
	acc_cfg.mwi_expires = PJSIP_MWI_DEFAULT_EXPIRES;
	acc_cfg.cred_count = 1;
	acc_cfg.cred_info[0].realm = pj_str((char *)domain);
	acc_cfg.cred_info[0].scheme = pj_str("digest");
	dave_snprintf((s8 *)impi_str, sizeof(impi_str), "%s@%s", impi, domain);
	acc_cfg.cred_info[0].username = pj_str(impi_str);
	acc_cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
	acc_cfg.cred_info[0].data = pj_str((char *)pwd);
	acc_cfg.transport_id = _transport_id = _pjsip_creat_transport(transport_type);
	if(_transport_id == -1)
	{
		PARTYABNOR("transport:%d creat failed!", transport_type);
		return -1;
	}
	acc_cfg.allow_contact_rewrite = PJ_TRUE;
	acc_cfg.contact_rewrite_method = PJSUA_CONTACT_REWRITE_METHOD;
	acc_cfg.contact_use_src_port = PJ_TRUE;
	acc_cfg.allow_via_rewrite = PJ_TRUE;
    acc_cfg.rtp_cfg.port = (unsigned)media_port;
    acc_cfg.rtp_cfg.port_range = (unsigned)media_port_range;

    status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &acc_id);
    if(status != PJ_SUCCESS)
    {
        PARTYABNOR("account add failed:%d", status);
        return -1;
    }

	if(acc_id > 0)
	{
		pjsua_acc_set_online_status(acc_id, PJ_TRUE);

		conf_subscribe_init(_owner_thread_id, acc_id);

		PARTYDEBUG("%s@%s connect %s:%d<%d> success!", user, domain, server_addr, server_port, transport_type);
	}

    return (sb)acc_id;
}

static dave_bool
_pjsip_account_logout(sb acc_id)
{
	pj_status_t status;

	if(acc_id < 0)
	{
		return dave_true;
	}

	_regc = NULL;

	conf_subscribe_exit();

	_pjsip_hangup_all();

	pjsua_acc_set_online_status((pjsua_acc_id)acc_id, PJ_FALSE);

	status = pjsua_acc_del((pjsua_acc_id)acc_id);

	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("acc_id:%d del failed:%d", acc_id, status);
		return dave_false;
	}

	return dave_true;
}

// =====================================================================

dave_bool
dave_pjsip_init(ThreadId owner, s8 *server_addr, ub server_port, AudioCodec codec[DAVE_ALLOWED_CODEC_MAX])
{
	dave_bool ret;

	if(_owner_thread_id == INVALID_THREAD_ID)
	{
		_owner_thread_id = owner;

		_regc = NULL;
		dave_memset(&_regc_timer, 0x00, sizeof(pj_timer_entry));

		_register_state_fun = NULL;
		_call_state_fun = NULL;

		ret = _pjsip_booting(server_addr, server_port, codec);
	}
	else
	{
		ret = ERRCODE_OK;
	}

	return ret;
}

void
dave_pjsip_exit(void)
{
	if(_owner_thread_id != INVALID_THREAD_ID)
	{
		_regc = NULL;

		_pjsip_thread_register();

		_pjsip_hangup_all();

		pjsua_destroy();

		_owner_thread_id = INVALID_THREAD_ID;
	}
}

void
dave_pjsip_reg_call_cb(pjsip_cb_fun cb_fun)
{
	_call_state_fun = cb_fun;
}

void
dave_pjsip_reg_incoming_cb(pjsip_cb_fun cb_fun)
{
	_incoming_state_fun = cb_fun;
}

void
dave_pjsip_reg_media_state_cb(pjsip_cb_fun cb_fun)
{
	_media_state_fun = cb_fun;
}

void
dave_pjsip_reg_incoming_message(pjsip_cb_fun cb_fun)
{
	_incoming_message_fun = cb_fun;
}

void
dave_pjsip_reg_message_state(pjsip_cb_fun cb_fun)
{
	_message_state_fun = cb_fun;
}

sb
dave_pjsip_login(s8 *server_addr, ub server_port, SIPTransportType transport_type,
	ub media_port, ub media_port_range,
	s8 *user, s8 *domain, s8 *pwd, s8 *impi,
	ub reg_timeout,
	pjsip_cb_fun register_fun)
{
	_register_state_fun = register_fun;

	return _pjsip_account_login(server_addr, server_port, transport_type, media_port, media_port_range, user, domain, pwd, impi, reg_timeout);
}

dave_bool
dave_pjsip_logout(sb acc_id)
{
	return _pjsip_account_logout(acc_id);
}

sb
dave_pjsip_make_call(sb acc_id, s8 *call_target)
{
	pj_str_t uri = pj_str((char *)call_target);
	pj_status_t status;
	pjsua_call_id call_id;

	status = pjsua_call_make_call((pjsua_acc_id)acc_id, &uri, 0, NULL, NULL, &call_id);

	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("invalid call:%s status:%d", call_target, status);
		return -1;
	}

	return (sb)call_id;
}

dave_bool
dave_pjsip_answer_call(sb call_id, SIPStatus code)
{
	pj_status_t status;

	status = pjsua_call_answer((pjsua_call_id)call_id, (unsigned)code, NULL, NULL);

	if(status != PJ_SUCCESS)
		return dave_false;
	else
		return dave_true;
}

dave_bool
dave_pjsip_hold_call(sb call_id)
{
	pj_status_t status;

	status = pjsua_call_set_hold((pjsua_call_id)call_id, NULL);
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("invalid hold:%d status:%d", call_id, status);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
dave_pjsip_unhold_call(sb call_id)
{
	pj_status_t status;

	status = pjsua_call_reinvite((pjsua_call_id)call_id, PJSUA_CALL_UNHOLD, NULL);
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("invalid unhold:%d status:%d", call_id, status);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

ErrCode
__dave_pjsip_hangup_call__(sb call_id, s8 *fun, ub line)
{
	pj_status_t status;

	PARTYDEBUG("call_id:%d %s:%d", call_id, fun, line);

	if(call_id < 0)
	{
		_pjsip_hangup_all();
		status = PJ_SUCCESS;
	}
	else
	{
		status = pjsua_call_hangup((pjsua_call_id)call_id, PJSIP_SC_GONE, NULL, NULL);
	}

	if(status == PJSIP_ESESSIONTERMINATED)
	{
		PARTYLOG("session:%d already terminated! %s:%d", call_id, fun, line);
		return ERRCODE_OK;
	}
	else if(status == PJ_SUCCESS)
	{
		return ERRCODE_wait;
	}
	else
	{
		PARTYABNOR("invalid hangup:%d status:%d %s:%d", call_id, status, fun, line);
		return ERRCODE_invalid_option;
	}	
}

dave_bool
dave_pjsip_refer_call(sb call_id, s8 *refer_to)
{
	pj_str_t pj_refer_to;
	pj_status_t status;

	pj_refer_to = pj_str((char *)refer_to);

	status = pjsua_call_xfer((pjsua_call_id)call_id, &pj_refer_to, NULL);
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("invalid refer:%d status:%d", call_id, status);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
dave_pjsip_update_call(sb call_id)
{
	pj_status_t status;

	status = pjsua_call_update((pjsua_call_id)call_id, PJSUA_CALL_NO_SDP_OFFER, NULL);
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("invalid refer:%d status:%d", call_id, status);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
dave_pjsip_conf_subscribe(s8 *request_uri, pjsip_cb_fun state_fun)
{
	return conf_subscribe(request_uri, state_fun);	
}

dave_bool
dave_pjsip_call_id_info(sb call_id, s8 *uri, s8 *id, s8 *to_tag, s8 *from_tag)
{
	pj_status_t status;
    pjsua_call *dest_call;
    pjsip_dialog *dest_dlg;
	pjsip_uri *dest_uri;
	const pjsip_parser_const_t *pconst;

	status = acquire_call("dave_sip_call_id_info()", (pjsua_call_id)call_id, &dest_call, &dest_dlg);
	if(status != PJ_SUCCESS)
	{
		return dave_false;
	}

	dest_uri = (pjsip_uri *)pjsip_uri_get_uri(dest_dlg->remote.info->uri);
	pjsip_uri_print(PJSIP_URI_IN_REQ_URI, dest_uri, (char *)uri, DAVE_URL_LEN);

	pconst = pjsip_parser_const();
	pj_strncpy2_escape((char *)id, &(dest_dlg->call_id->id), DAVE_URL_LEN, &(pconst->pjsip_HDR_CHAR_SPEC));

	dave_memcpy(to_tag, dest_dlg->remote.info->tag.ptr, dest_dlg->remote.info->tag.slen);
	dave_memcpy(from_tag, dest_dlg->local.info->tag.ptr, dest_dlg->local.info->tag.slen);

	return dave_true;
}

void
dave_pjsip_register_option(dave_bool on)
{
	if(on == dave_true)
		_pjsip_start_register_timer();
	else
		_pjsip_stop_register_timer();
}

dave_bool
dave_pjsip_set_mute_mode(dave_bool mute, sb call_id)
{
    pj_status_t status;
    pjsua_call_info ci;
    pjsua_call_get_info((pjsua_call_id)call_id, &ci);
    if (ci.conf_slot <= 0) {
        return dave_false;
    }
    if (mute) {
        status = pjsua_conf_disconnect(0,ci.conf_slot);
    }else{
        status =  pjsua_conf_connect(0, ci.conf_slot);
    }
    if (status != PJ_SUCCESS) {
        return dave_false;
    }
    return dave_true;
}

dave_bool
dave_pjsip_send_im(sb acc_id, s8 *target, s8 *mime_type, s8 *content)
{
    pj_status_t status;
    pj_str_t im_to = pj_str((char *)target);
    pj_str_t im_mime_type = pj_str((char *)mime_type);
    pj_str_t im_content = pj_str((char *)content);

    status = pjsua_im_send((pjsua_acc_id)acc_id, &im_to, &im_mime_type, &im_content, NULL, NULL);
    
    if (status != PJ_SUCCESS) {
        return dave_false;
    }
    return dave_true;
}

#endif

