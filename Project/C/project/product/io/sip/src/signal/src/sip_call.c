/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "dave_rtp.h"
#include "sip_signal.h"
#include "sip_call_class.h"
#include "sip_log.h"

static void
_sip_call_rtp_release(SIPCall *pCall)
{
	if(pCall->rtp != NULL)
	{
		dave_rtp_release(pCall->rtp);
		pCall->rtp = NULL;
	}
}

static dave_bool
_sip_call_rtp_creat(
	SIPSignal *pSignal, SIPCall *pCall,
	rtp_data_start data_start, rtp_data_stop data_stop, rtp_data_recv data_recv)
{
	_sip_call_rtp_release(pCall);

	pCall->rtp = dave_rtp_creat(
		pSignal->rtp_ip, pSignal->rtp_port,
		data_start, data_stop, data_recv,
		pCall);

	if(pCall->rtp == NULL)
		return dave_false;

	return dave_true;
}

static void
_sip_call_update(SIPCall *pCall, osip_message_t *sip)
{
	if(sip->sip_method != NULL)
		return;

	if((sip->status_code != SIP_TRYING) && (sip->status_code != SIP_RINGING) && (sip->status_code != SIP_SESSION_PROGRESS))
		return;

	if(osip_cseq_match(pCall->cseq, sip->cseq) != OSIP_SUCCESS)
	{
		SIPABNOR("cseq mismatch:%s %s/%s %s",
			pCall->cseq->method, pCall->cseq->number,
			sip->cseq->method, sip->cseq->number);
		return;
	}

	if(sip->call_id != NULL)
	{
		if(pCall->call_id == NULL)
		{
			osip_call_id_clone(sip->call_id, &(pCall->call_id));
		}
		else
		{
			if(sip->call_id->number != NULL)
			{
				if(dave_strcmp(pCall->call_id->number, sip->call_id->number) != dave_true)
				{
					SIPABNOR("call_id:%s/%s mismatch!", pCall->call_id->number, sip->call_id->number);
					return;
				}
			}
		}
	}

	if(sip->from != NULL)
	{
		if(pCall->from == NULL)
		{
			osip_from_clone(sip->from, &(pCall->from));
		}
		else
		{
			if(dave_strcmp(pCall->from->url->username, sip->from->url->username) != dave_true)
			{
				SIPABNOR("from username:%s/%s mismatch!", pCall->from->url->username, sip->from->url->username)
				return;
			}
			if(dave_strcmp(pCall->from->url->host, sip->from->url->host) != dave_true)
			{
				SIPABNOR("from host:%s/%s mismatch!", pCall->from->url->host, sip->from->url->host)
				return;
			}
			osip_uri_param_t *call_from_tag, *sip_from_tag;

			osip_from_get_tag(pCall->from, &call_from_tag);
			osip_from_get_tag(sip->from, &sip_from_tag);

			if(call_from_tag == NULL)
			{
				osip_from_free(pCall->from);
				pCall->from = NULL;
				osip_from_clone(sip->from, &(pCall->from));
			}
			else
			{
				if(dave_strcmp(call_from_tag->gvalue, sip_from_tag->gvalue) != dave_true)
				{
					SIPABNOR("from tag:%s/%s mismatch!", call_from_tag->gvalue, sip_from_tag->gvalue)
					return;
				}
			}
		}
	}

	if(sip->to != NULL)
	{
		if(pCall->to == NULL)
		{
			osip_to_clone(sip->to, &(pCall->to));
		}
		else
		{
			if(dave_strcmp(pCall->to->url->username, sip->to->url->username) != dave_true)
			{
				SIPABNOR("to username:%s/%s mismatch!", pCall->to->url->username, sip->to->url->username)
				return;
			}
			if(dave_strcmp(pCall->to->url->host, sip->to->url->host) != dave_true)
			{
				SIPABNOR("to host:%s/%s mismatch!", pCall->to->url->host, sip->to->url->host)
				return;
			}
			osip_uri_param_t *call_to_tag, *sip_to_tag;

			osip_to_get_tag(pCall->to, &call_to_tag);
			osip_to_get_tag(sip->to, &sip_to_tag);

			if(call_to_tag == NULL)
			{
				osip_to_free(pCall->to);
				pCall->to = NULL;
				osip_to_clone(sip->to, &(pCall->to));
			}
			else
			{
				if(dave_strcmp(call_to_tag->gvalue, sip_to_tag->gvalue) != dave_true)
				{
					SIPABNOR("to tag:%s/%s mismatch!", call_to_tag->gvalue, sip_to_tag->gvalue)
					return;
				}
			}
		}
	}
}

static SIPCall *
_sip_call_build(SIPSignal *pSignal, SIPCall *pCall, osip_message_t *sip)
{
	sip_call_creat(pSignal, osip_get_call_id(sip), pCall);

	osip_call_id_clone(sip->call_id, &(pCall->call_id));
	osip_from_clone(sip->from, &(pCall->from));
	osip_to_clone(sip->to, &(pCall->to));
	osip_cseq_clone(sip->cseq, &(pCall->cseq));

	return pCall;
}

static void
_sip_call_start(SIPCall *pCall)
{
	SIPSignal *pSignal = (SIPSignal *)(pCall->signal);

	SIPLOG("server:%s:%s user:%s/%s local:%s:%s rtp:%s:%s",
		pSignal->server_ip, pSignal->server_port,
		pSignal->username, pSignal->password,
		pSignal->local_ip, pSignal->local_port,
		pSignal->rtp_ip, pSignal->rtp_port);

	if(pCall->start_fun != NULL)
	{
		pCall->start_fun(pCall);
	}
	else
	{
		SIPABNOR("call:%s bye_fun is NULL!", pCall->call_data);
	}
}

static void
_sip_call_end(SIPCall *pCall)
{
	SIPSignal *pSignal = (SIPSignal *)(pCall->signal);

	SIPLOG("server:%s:%s user:%s/%s local:%s:%s rtp:%s:%s",
		pSignal->server_ip, pSignal->server_port,
		pSignal->username, pSignal->password,
		pSignal->local_ip, pSignal->local_port,
		pSignal->rtp_ip, pSignal->rtp_port);

	_sip_call_rtp_release(pCall);

	if(pCall->end_fun != NULL)
	{
		pCall->end_fun(pCall);
	}
	else
	{
		SIPABNOR("call:%s bye_fun is NULL!", pCall->call_data);
	}

	sip_call_release(pSignal, pCall);
}

static void
_sip_call_sdp_bind(SIPCall *pCall, osip_message_t *pRecv)
{
	sdp_message_t *sdp;
	sdp_media_t *med;

	if((pRecv->sip_method != NULL) && (pRecv->status_code != 183) && (pRecv->status_code != 200))
		return;

	sdp = osip_load_sdp(pRecv);
	if(sdp == NULL)
	{
		return;
	}

	med = osip_list_get(&sdp->m_medias, 0);

	SIPLOG("c_nettype:%s c_addrtype:%s c_addr:%s multicast:%s/%s media:%s/%s/%s",
		sdp->c_connection->c_nettype, sdp->c_connection->c_addrtype,
		sdp->c_connection->c_addr,
		sdp->c_connection->c_addr_multicast_ttl, sdp->c_connection->c_addr_multicast_int,
		med->m_media, med->m_port, med->m_proto);

	if(pCall->rtp != NULL)
	{
		SIPLOG("rtp is creat! remote:%s:%s local:%s",
			pCall->rtp->remote_rtp_ip, pCall->rtp->remote_rtp_port,
			pCall->rtp->local_rtp_port);

		dave_rtp_send_build(pCall->rtp, sdp->c_connection->c_addr, med->m_port);
	}
	else
	{
		SIPLOG("rtp is NULL!!!!!");
	}

	sdp_message_free(sdp);
}

static ub
_sip_call_recv(void *signal, osip_message_t *pRecv)
{
	SIPSignal *pSignal = (SIPSignal *)signal;
	SIPCall *pCall = sip_call_id_query(pSignal, osip_get_call_id(pRecv));

	if(pCall == NULL)
	{
		SIPLOG("%s->%s %s can't find the call!",
			osip_get_from_user(pRecv), osip_get_to_user(pRecv),
			osip_get_call_id(pRecv));
		return SIP_NOT_FOUND;
	}

	if(pRecv->sip_method == NULL)
	{
		_sip_call_update(pCall, pRecv);

		switch(pRecv->status_code)
		{
			case SIP_TRYING:
			case SIP_RINGING:
				break;
			case SIP_SESSION_PROGRESS:
					_sip_call_sdp_bind(pCall, pRecv);
				break;
			case SIP_OK:
					_sip_call_start(pCall);
				break;
			case SIP_TEMPORARILY_UNAVAILABLE:
			case SIP_REQUEST_TERMINATED:
			case SIP_SERVICE_UNAVAILABLE:
					_sip_call_end(pCall);
				break;
			default:
					SIPABNOR("unprocess status:%d %s", pRecv->status_code, pRecv->reason_phrase);
				break;
		}
	}
	else
	{
		if(dave_strcmp(pRecv->sip_method, "ACK") == dave_true)
		{
			SIPLOG("method:%s", pRecv->sip_method);
		}
		else if(dave_strcmp(pRecv->sip_method, "BYE") == dave_true)
		{
			_sip_call_end(pCall);
		}
		else
		{
			SIPABNOR("unprocess method:%s", pRecv->sip_method);
		}
	}

	return SIP_OK;
}

static ub
_sip_bye_recv(void *signal, osip_message_t *pRecv)
{
	SIPSignal *pSignal = (SIPSignal *)signal;
	SIPCall *pCall = sip_call_id_query(pSignal, osip_get_call_id(pRecv));

	if(pCall == NULL)
	{
		SIPLOG("%s->%s %s can't find the call!",
			osip_get_from_user(pRecv), osip_get_to_user(pRecv),
			osip_get_call_id(pRecv));
		return SIP_NOT_FOUND;
	}

	if(pRecv->sip_method == NULL)
	{
		switch(pRecv->status_code)
		{
			case SIP_OK:
			case SIP_TEMPORARILY_UNAVAILABLE:
			case SIP_REQUEST_TERMINATED:
			case SIP_SERVICE_UNAVAILABLE:
					_sip_call_end(pCall);
				break;
			default:
					SIPABNOR("unprocess status:%d %s", pRecv->status_code, pRecv->reason_phrase);
				break;
		}
	}
	else
	{
		if(dave_strcmp(pRecv->sip_method, "ACK") == dave_true)
		{
			SIPLOG("method:%s", pRecv->sip_method);
		}
		else if(dave_strcmp(pRecv->sip_method, "BYE") == dave_true)
		{
			_sip_call_end(pCall);
		}
		else
		{
			SIPABNOR("unprocess method:%s", pRecv->sip_method);
		}
	}

	return SIP_OK;
}

static dave_bool
_sip_call_send(
	SIPSignal *pSignal, SIPCall *pCall,
	rtp_data_start data_start, rtp_data_stop data_stop, rtp_data_recv data_recv)
{
	osip_message_t *sip;

	if(_sip_call_rtp_creat(pSignal, pCall, data_start, data_stop, data_recv) == dave_false)
	{
		SIPLOG("phone_number:%s call failed!", pCall->call_data);
		return dave_false;
	}

	SAFECODEv1(pSignal->request_pv, {
		sip = osip_invite(
			pSignal->server_ip, pSignal->server_port, pSignal->username,
			pSignal->local_ip, pSignal->local_port,
			pCall->rtp->local_rtp_ip, pCall->rtp->local_rtp_port,
			pCall->call_data, pSignal->cseq_number ++);
	});

	SIPLOG("server:%s:%s local:%s:%s rtp:%s:%s phone_number:%s call_id:%s cseq:%s/%s",
		pSignal->server_ip, pSignal->server_port,
		pSignal->local_ip, pSignal->local_port,
		pCall->rtp->local_rtp_ip, pCall->rtp->local_rtp_port,
		pCall->call_data, sip->call_id->number, sip->cseq->method, sip->cseq->number);

	dave_rtp_call_id_build(pCall->rtp, osip_call_id_get_number(sip->call_id), pSignal->username, pCall->call_data);

	sip_signal_reg_inv(pSignal, _sip_call_recv, pSignal);
	sip_signal_reg_bye(pSignal, _sip_bye_recv, pSignal);

	_sip_call_build(pSignal, pCall, sip);

	sip_signal_send(pSignal, sip);

	return dave_true;
}

static void
_sip_bye_send(SIPSignal *pSignal, SIPCall *pCall)
{
	osip_message_t *sip;

	SAFECODEv1(pSignal->request_pv, {
		sip = osip_bye(
			pSignal->server_ip, pSignal->server_port, pSignal->username,
			pSignal->local_ip, pSignal->local_port,
			pCall->from, pCall->to, pCall->call_id,
			pSignal->cseq_number ++);
	});

	sip_signal_send(pSignal, sip);

	_sip_call_rtp_release(pCall);
}

// =====================================================================

SIPCall *
sip_call(
	SIPSignal *pSignal, s8 *call_data,
	rtp_data_start data_start, rtp_data_stop data_stop, rtp_data_recv data_recv,
	call_start_fun start_fun, call_end_fun end_fun,
	void *user_ptr)
{
	SIPCall *pCall = sip_call_data_query(pSignal, call_data);

	if(pCall != NULL)
	{
		SIPLOG("call:%s resource conflicts!", call_data);
		return NULL;
	}

	pCall = sip_call_build(pSignal, call_data);

	dave_strcpy(pCall->call_data, call_data, sizeof(pCall->call_data));

	pCall->start_fun = start_fun;
	pCall->end_fun = end_fun;
	
	pCall->user_ptr = user_ptr;

	SIPLOG("call_data:%s", pCall->call_data);

	if(_sip_call_send(pSignal, pCall, data_start, data_stop, data_recv) == dave_false)
	{
		SIPLOG("call:%s failed!", call_data);
		sip_call_release(pSignal, pCall);
		return NULL;
	}

	return pCall;
}

RetCode
sip_bye(SIPSignal *pSignal, s8 *call_data)
{
	SIPCall *pCall = sip_call_data_query(pSignal, call_data);

	if(pCall == NULL)
	{
		SIPLOG("call_data:%s is bye!", call_data);
	}
	else
	{	
		_sip_bye_send(pSignal, pCall);
	}

	return RetCode_OK;
}

SIPCall *
sip_my_call(SIPSignal *pSignal, s8 *call_id)
{
	return sip_call_id_query(pSignal, call_id);
}

