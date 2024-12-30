/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "uac_main.h"
#include "uac_reg.h"
#include "uac_cfg.h"
#include "uac_log.h"

static void
_uac_call_rtp_release(UACClass *pClass)
{
	if(pClass->call.rtp != NULL)
	{
		uac_rtp_release(pClass->call.rtp);
		pClass->call.rtp = NULL;
	}
}

static dave_bool
_uac_call_rtp_creat(UACClass *pClass)
{
	_uac_call_rtp_release(pClass);

	pClass->call.rtp = uac_rtp_creat(pClass->signal.rtp_ip, pClass->signal.rtp_port);

	if(pClass->call.rtp == NULL)
		return dave_false;

	return dave_true;
}

static void
_uac_call_release(UACClass *pClass)
{
	dave_memset(pClass->call.phone_number, 0x00, sizeof(pClass->call.phone_number));

	if(pClass->call.call_id != NULL)
	{
		osip_call_id_free(pClass->call.call_id);
		pClass->call.call_id = NULL;
	}

	if(pClass->call.from != NULL)
	{
		osip_from_free(pClass->call.from);
		pClass->call.from = NULL;
	}

	if(pClass->call.to != NULL)
	{
		osip_to_free(pClass->call.to);
		pClass->call.to = NULL;
	}

	_uac_call_rtp_release(pClass);
}

static UACCall *
_uac_call_creat(UACClass *pClass)
{
	_uac_call_release(pClass);

	return &(pClass->call);
}

static void
_uac_call_update(UACClass *pClass, osip_message_t *sip)
{
	if(sip->sip_method != NULL)
		return;

	if((sip->status_code != SIP_TRYING) && (sip->status_code != SIP_RINGING) && (sip->status_code != SIP_SESSION_PROGRESS))
		return;

	if(osip_cseq_match(pClass->call.cseq, sip->cseq) != OSIP_SUCCESS)
	{
		UACABNOR("cseq mismatch:%s %s/%s %s",
			pClass->call.cseq->method, pClass->call.cseq->number,
			sip->cseq->method, sip->cseq->number);
		return;
	}

	if(sip->call_id != NULL)
	{
		if(pClass->call.call_id == NULL)
		{
			osip_call_id_clone(sip->call_id, &(pClass->call.call_id));
		}
		else
		{
			if(sip->call_id->number != NULL)
			{
				if(dave_strcmp(pClass->call.call_id->number, sip->call_id->number) != dave_true)
				{
					UACABNOR("call_id:%s/%s mismatch!", pClass->call.call_id->number, sip->call_id->number);
					return;
				}
			}
		}
	}

	if(sip->from != NULL)
	{
		if(pClass->call.from == NULL)
		{
			osip_from_clone(sip->from, &(pClass->call.from));
		}
		else
		{
			if(dave_strcmp(pClass->call.from->url->username, sip->from->url->username) != dave_true)
			{
				UACABNOR("from username:%s/%s mismatch! ", pClass->call.from->url->username, sip->from->url->username)
				return;
			}
			if(dave_strcmp(pClass->call.from->url->host, sip->from->url->host) != dave_true)
			{
				UACABNOR("from host:%s/%s mismatch! ", pClass->call.from->url->host, sip->from->url->host)
				return;
			}
			osip_uri_param_t *call_from_tag, *sip_from_tag;

			osip_from_get_tag(pClass->call.from, &call_from_tag);
			osip_from_get_tag(sip->from, &sip_from_tag);

			if(call_from_tag == NULL)
			{
				osip_from_free(pClass->call.from);
				pClass->call.from = NULL;
				osip_from_clone(sip->from, &(pClass->call.from));
			}
			else
			{
				if(dave_strcmp(call_from_tag->gvalue, sip_from_tag->gvalue) != dave_true)
				{
					UACABNOR("from tag:%s/%s mismatch! ", call_from_tag->gvalue, sip_from_tag->gvalue)
					return;
				}
			}
		}
	}

	if(sip->to != NULL)
	{
		if(pClass->call.to == NULL)
		{
			osip_to_clone(sip->to, &(pClass->call.to));
		}
		else
		{
			if(dave_strcmp(pClass->call.to->url->username, sip->to->url->username) != dave_true)
			{
				UACABNOR("to username:%s/%s mismatch! ", pClass->call.to->url->username, sip->to->url->username)
				return;
			}
			if(dave_strcmp(pClass->call.to->url->host, sip->to->url->host) != dave_true)
			{
				UACABNOR("to host:%s/%s mismatch! ", pClass->call.to->url->host, sip->to->url->host)
				return;
			}
			osip_uri_param_t *call_to_tag, *sip_to_tag;

			osip_to_get_tag(pClass->call.to, &call_to_tag);
			osip_to_get_tag(sip->to, &sip_to_tag);

			if(call_to_tag == NULL)
			{
				osip_to_free(pClass->call.to);
				pClass->call.to = NULL;
				osip_to_clone(sip->to, &(pClass->call.to));
			}
			else
			{
				if(dave_strcmp(call_to_tag->gvalue, sip_to_tag->gvalue) != dave_true)
				{
					UACABNOR("to tag:%s/%s mismatch! ", call_to_tag->gvalue, sip_to_tag->gvalue)
					return;
				}
			}
		}
	}
}

static void
_uac_call_build(UACClass *pClass, osip_message_t *sip)
{
	if(pClass->call.call_id != NULL)
	{
		osip_call_id_free(pClass->call.call_id);
	}
	if(pClass->call.from != NULL)
	{
		osip_from_free(pClass->call.from);
	}
	if(pClass->call.to != NULL)
	{
		osip_to_free(pClass->call.to);
	}
	if(pClass->call.cseq != NULL)
	{
		osip_cseq_free(pClass->call.cseq);
	}

	osip_call_id_clone(sip->call_id, &(pClass->call.call_id));
	osip_from_clone(sip->from, &(pClass->call.from));
	osip_to_clone(sip->to, &(pClass->call.to));
	osip_cseq_clone(sip->cseq, &(pClass->call.cseq));
}

static void
_uac_call_end(UACClass *pClass)
{
	UACLOG("server:%s:%s user:%s/%s local:%s:%s rtp:%s:%s",
		pClass->signal.server_ip, pClass->signal.server_port,
		pClass->signal.username, pClass->signal.password,
		pClass->signal.local_ip, pClass->signal.local_port,
		pClass->signal.rtp_ip, pClass->signal.rtp_port);

	_uac_call_rtp_release(pClass);
	
	_uac_call_release(pClass);
}

static void
_uac_call_sdp_bind(UACClass *pClass, osip_message_t *pRecv)
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

	UACLOG("c_nettype:%s c_addrtype:%s c_addr:%s multicast:%s/%s media:%s/%s/%s",
		sdp->c_connection->c_nettype, sdp->c_connection->c_addrtype,
		sdp->c_connection->c_addr,
		sdp->c_connection->c_addr_multicast_ttl, sdp->c_connection->c_addr_multicast_int,
		med->m_media, med->m_port, med->m_proto);

	if(pClass->call.rtp != NULL)
	{
		UACLOG("rtp is creat! remote:%s:%s local:%s",
			pClass->call.rtp->remote_rtp_ip, pClass->call.rtp->remote_rtp_port,
			pClass->call.rtp->local_rtp_port);

		uac_rtp_send_build(pClass->call.rtp, sdp->c_connection->c_addr, med->m_port);
	}
	else
	{
		UACLOG("rtp is NULL!!!!!");
	}

	sdp_message_free(sdp);
}

static ub
_uac_call_recv(void *class, osip_message_t *pRecv)
{
	UACClass *pClass = (UACClass *)class;

	if(pRecv->sip_method == NULL)
	{
		_uac_call_update(pClass, pRecv);

		switch(pRecv->status_code)
		{
			case SIP_TRYING:
			case SIP_RINGING:
				break;
			case SIP_SESSION_PROGRESS:
					_uac_call_sdp_bind(pClass, pRecv);
				break;
			case SIP_OK:
				break;
			case SIP_TEMPORARILY_UNAVAILABLE:
			case SIP_REQUEST_TERMINATED:
			case SIP_SERVICE_UNAVAILABLE:
					_uac_call_end(pClass);
				break;
			default:
					UACABNOR("unprocess status:%d %s", pRecv->status_code, pRecv->reason_phrase);
				break;
		}
	}
	else
	{
		if(dave_strcmp(pRecv->sip_method, "ACK") == dave_true)
		{
			UACLOG("method:%s", pRecv->sip_method);
		}
		else if(dave_strcmp(pRecv->sip_method, "BYE") == dave_true)
		{
			_uac_call_end(pClass);
		}
		else
		{
			UACABNOR("unprocess method:%s", pRecv->sip_method);
		}
	}

	return SIP_OK;
}

static void
_uac_call_send(UACClass *pClass, s8 *phone_number)
{
	osip_message_t *sip;

	if(_uac_call_rtp_creat(pClass) == dave_false)
	{
		UACLOG("phone_number:%s call failed!", phone_number);
		return;
	}

	SAFECODEv1(pClass->signal.request_pv, {
		sip = osip_invite(
			pClass->signal.server_ip, pClass->signal.server_port, pClass->signal.username,
			pClass->signal.local_ip, pClass->signal.local_port,
			pClass->call.rtp->local_rtp_ip, pClass->call.rtp->local_rtp_port,
			phone_number, pClass->signal.cseq_number ++);
	});

	UACLOG("server:%s:%s local:%s:%s rtp:%s:%s phone_number:%s call_id:%s cseq:%s/%s",
		pClass->signal.server_ip, pClass->signal.server_port,
		pClass->signal.local_ip, pClass->signal.local_port,
		pClass->call.rtp->local_rtp_ip, pClass->call.rtp->local_rtp_port,
		phone_number, sip->call_id->number, sip->cseq->method, sip->cseq->number);

	uac_rtp_call_id_build(pClass->call.rtp, sip->call_id->number, pClass->signal.username, phone_number);

	uac_class_reg_inv(pClass, _uac_call_recv);
	uac_class_reg_bye(pClass, _uac_call_recv);

	_uac_call_build(pClass, sip);

	uac_class_send(pClass, sip);

	dave_strcpy(pClass->call.phone_number, phone_number, sizeof(pClass->call.phone_number));
}

static void
_uac_bye_send(UACClass *pClass)
{
	osip_message_t *sip;

	SAFECODEv1(pClass->signal.request_pv, {
		sip = osip_bye(
			pClass->signal.server_ip, pClass->signal.server_port, pClass->signal.username,
			pClass->signal.local_ip, pClass->signal.local_port,
			pClass->call.from, pClass->call.to, pClass->call.call_id,
			pClass->signal.cseq_number ++);
	});

	uac_class_send(pClass, sip);

	_uac_call_rtp_release(pClass);
}

// =====================================================================

void
uac_call_init(void)
{

}

void
uac_call_exit(void)
{

}

void
uac_bye(void)
{
	UACClass *pClass = uac_main_class();

	UACLOG("phone_number:%s", pClass->call.phone_number);

	_uac_bye_send(pClass);
}

void
uac_call(s8 *phone_number)
{
	UACClass *pClass = uac_main_class();

	UACLOG("phone_number:%s", phone_number);

	if(dave_strcmp(pClass->call.phone_number, phone_number) == dave_true)
	{
		UACLOG("phone_number:%s is calling!", phone_number);
		return;
	}
	
	if(pClass->call.phone_number[0] != '\0')
	{
		UACLOG("on call state:%s", pClass->call.phone_number);
		return;
	}

	_uac_call_creat(pClass);

	_uac_call_send(pClass, phone_number);
}

