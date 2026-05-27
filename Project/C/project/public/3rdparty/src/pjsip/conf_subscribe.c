/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_3rdparty.h"
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
#include "party_log.h"
#include "pjsua.h"
#include "pjsip.h"
#include <pjsua-lib/pjsua_internal.h>
#include <pjmedia/transport.h>
#include "pjsip_endpoint.h"
#include "pjsip_param.h"

#define SUBSCRIBE_DEFAULT_EXPIRES 3600

static const pj_str_t _str_conference_event = { "conference", 10 };

static const pj_str_t _xml_conference_state = { "conference-state", 16 };
static const pj_str_t _xml_user_count = { "user-count", 10 };
static const pj_str_t _xml_users = { "users", 5 };
static const pj_str_t _xml_user = { "user", 4 };
static const pj_str_t _xml_endpoint = { "endpoint", 8 };
static const pj_str_t _xml_entity = { "entity",  6 };
static const pj_str_t _xml_status = { "status", 6 };

static ThreadId _owner_thread_id = INVALID_THREAD_ID;
static SubscribeModData _conf_mod_data;
static SIPNotifyInfo _conf_subscribe_info;
static pjsip_cb_fun _conf_subscribe_cb = NULL;

static struct pjsip_module _mod_subscribe = {
    NULL, NULL,					/* prev, next. */
    { "mod-conf-subscribe", 18 },	/* Name. */
    -1,							/* Id	*/
    PJSIP_MOD_PRIORITY_DIALOG_USAGE,/* Priority				*/
    NULL,			    /* load()				*/
    NULL,			    /* start()				*/
    NULL,			    /* stop()				*/
    NULL,			    /* unload()				*/
    NULL,			    /* on_rx_request()			*/
    NULL,			    /* on_rx_response()			*/
    NULL,			    /* on_tx_request.			*/
    NULL,			    /* on_tx_response()			*/
    NULL,			    /* on_tsx_state()			*/
};

static SubscribeModData *
_conf_subscribe_get_mod_data(pjsip_evsub *sub)
{
	if(_mod_subscribe.id >= 0)
	{
		return (SubscribeModData *)pjsip_evsub_get_mod_data(sub, _mod_subscribe.id);
	}
	else
	{
		return NULL;
	}
}

static void
_conf_subscribe_notify_owner(pjsip_cb_fun fun, SIPNotifyInfo *pInfo)
{

}

static ub
_conf_subscribe_find_user_count(pj_xml_node *doc)
{
	pj_xml_node *conference_state_doc, *user_count_doc;
	ub count = 0;
	s8 count_str[128];
	ub count_str_cpy_len;

	conference_state_doc = pj_xml_find_node(doc, &_xml_conference_state);
	if(conference_state_doc != NULL)
	{
		user_count_doc = pj_xml_find_node(conference_state_doc, &_xml_user_count);
		if(user_count_doc != NULL)
		{
			if(user_count_doc->content.slen >= sizeof(count_str))
			{
				count_str_cpy_len = sizeof(count_str);
			}
			else
			{
				count_str_cpy_len = (ub)(user_count_doc->content.slen + 1);
			}
			dave_strcpy(count_str, (s8 *)(user_count_doc->content.ptr), count_str_cpy_len);

			count = stringdigital(count_str);
		}
	}

	return count;
}

static dave_bool
_conf_subscribe_find_endpoint(pj_xml_node *user, SIPConfInfo *pConfInfo)
{
	pj_xml_node *endpoint, *status;
	pj_xml_attr *entity_attr;
	ub cpy_len;
	s8 status_str[128];

	endpoint = pj_xml_find_node(user, &_xml_endpoint);
	if(endpoint == NULL)
		return dave_false;

	entity_attr = pj_xml_find_attr(endpoint, &_xml_entity, NULL);
	if(entity_attr == NULL)
		return dave_false;

	if(entity_attr->value.slen == 0)
		return dave_false;

	if(entity_attr->value.slen >= DAVE_URL_LEN)
		cpy_len = DAVE_URL_LEN;
	else
		cpy_len = entity_attr->value.slen + 1;
	dave_strcpy(pConfInfo->uri, (s8 *)(entity_attr->value.ptr), cpy_len);

	status = pj_xml_find_node(endpoint, &_xml_status);
	if(status == NULL)
		return dave_false;

	if(status->content.slen >= sizeof(status_str))
		cpy_len = sizeof(status_str);
	else
		cpy_len = status->content.slen + 1;
	dave_strcpy(status_str, (s8 *)(status->content.ptr), cpy_len);
	if(dave_strcmp(status_str, (s8 *)"connected") == dave_false)
	{
		PARTYABNOR("invalid status:%s", status_str);
		pConfInfo->state = SIPStatus_DISCONNECTED;
	}
	else
	{
		pConfInfo->state = SIPStatus_CONFIRMED;
	}

	return dave_true;
}

static dave_bool
_conf_subscribe_find_users(pj_xml_node *doc, ub user_count, SIPNotifyInfo *pInfo)
{
	pj_xml_node *users, *user;
	ub user_index;

	if((doc == NULL) || (user_count == 0))
	{
		pInfo->conf_user_number = 0;

		return dave_true;
	}

	users = pj_xml_find_node(doc, &_xml_users);
	if(users == NULL)
	{
		return dave_false;
	}

	user = NULL;

	for(user_index=0; user_index<user_count; user_index++)
	{
		if(user == NULL)
		{
			user = pj_xml_find_node(users, &_xml_user);
		}
		else
		{
			user = pj_xml_find_next_node(users, user, &_xml_user);
		}

		if(user == NULL)
		{
			break;
		}

		if(_conf_subscribe_find_endpoint(user, &(pInfo->conf_user[user_index])) == dave_false)
		{
			break;
		}
	}

	_conf_subscribe_info.conf_user_number = user_index;

	if(_conf_subscribe_info.conf_user_number == 0)
		return dave_false;
	else
		return dave_true;
}

static void
_conf_subscribe_notify_info(pj_xml_node *doc)
{
	ub user_count;

	_conf_subscribe_info.state = SIPStatus_CONF_NOTIFY;
	_conf_subscribe_info.conf_user_number = 0;

	if(doc != NULL)
	{
		user_count = _conf_subscribe_find_user_count(doc);
		if(user_count > DAVE_CONF_USER_MAX)
		{
			PARTYABNOR("too many conf_user_number:%d", user_count);
			user_count = DAVE_CONF_USER_MAX;
		}
	}
	else
	{
		user_count = 0;
	}

	if(_conf_subscribe_find_users(doc, user_count, &_conf_subscribe_info) == dave_true)
	{
		_conf_subscribe_notify_owner(_conf_subscribe_cb, &_conf_subscribe_info);
	}
	else
	{
		PARTYABNOR("can't find user?");
	}
}

static void
_conf_subscribe_on_evsub_state(pjsip_evsub *sub, pjsip_event *event)
{
	SubscribeModData *pData = _conf_subscribe_get_mod_data(sub);

	if((pData != NULL) && (pjsip_evsub_get_state(sub) == PJSIP_EVSUB_STATE_TERMINATED))
	{
		pData->dlg = NULL;
		pData->sub = NULL;
	}
}

static void
_conf_subscribe_on_evsub_tsx_state(pjsip_evsub *sub, pjsip_transaction *tsx, pjsip_event *event)
{
	SubscribeModData *pData = _conf_subscribe_get_mod_data(sub);

	if((pData != NULL) && (pjsip_evsub_get_state(sub) == PJSIP_EVSUB_STATE_TERMINATED))
	{
		pData->dlg = NULL;
		pData->sub = NULL;
	}
}

static void
_conf_subscribe_on_evsub_rx_refresh(pjsip_evsub *sub, pjsip_rx_data *rdata, int *p_st_code, pj_str_t **p_st_text, pjsip_hdr *res_hdr, pjsip_msg_body **p_body)
{
	SubscribeModData *pData = _conf_subscribe_get_mod_data(sub);

	if((pData != NULL) && (pjsip_evsub_get_state(sub) == PJSIP_EVSUB_STATE_TERMINATED))
	{
		pData->dlg = NULL;
		pData->sub = NULL;
	}
}

static void
_conf_subscribe_on_evsub_rx_notify(pjsip_evsub *sub, pjsip_rx_data *rdata, int *p_st_code, pj_str_t **p_st_text, pjsip_hdr *res_hdr, pjsip_msg_body **p_body)
{
	pjsip_msg_body *body = NULL;
	pj_xml_node *doc = NULL;

	if((rdata != NULL) && (rdata->msg_info.msg != NULL) && (rdata->msg_info.msg->body != NULL))
	{
		body = rdata->msg_info.msg->body;
	}

	if(body != NULL)
	{
		doc = pj_xml_parse(rdata->tp_info.pool, (char *)body->data, body->len);
		if(doc != NULL)
		{
			_conf_subscribe_notify_info(doc);
		}
	}
	else
	{
		_conf_subscribe_notify_info(NULL);
	}
}

static void
_conf_subscribe_on_evsub_client_refresh(pjsip_evsub *sub)
{

}

static void
_conf_subscribe_on_evsub_server_timeout(pjsip_evsub *sub)
{

}

static pjsip_evsub_user _evsub_subscribe_user = {
	&_conf_subscribe_on_evsub_state,
 	&_conf_subscribe_on_evsub_tsx_state,
	&_conf_subscribe_on_evsub_rx_refresh,
	&_conf_subscribe_on_evsub_rx_notify,
	&_conf_subscribe_on_evsub_client_refresh,
	&_conf_subscribe_on_evsub_server_timeout,
};

static void
_conf_subscribe_unregister_mode(pjsip_module *pModule)
{
	pjsip_endpt_unregister_module(pjsua_var.endpt, pModule);
}

static dave_bool
_conf_subscribe_register_mode(pjsip_module *pModule)
{
	pj_status_t status;
	pj_str_t accept = { "application/conference-info+xml", 31 };

	status = pjsip_endpt_register_module(pjsua_var.endpt, pModule);
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("register module failed!");
		return dave_false;
	}

	status = pjsip_evsub_register_pkg(pModule, &_str_conference_event, SUBSCRIBE_DEFAULT_EXPIRES, 1, &accept);
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("register pkg failed!");
		_conf_subscribe_unregister_mode(pModule);
		return dave_false;
	}

	return dave_true;
}

static dave_bool
_conf_subscribe_register_data(pjsip_module *pModule, SubscribeModData *pData)
{
	pjsua_acc *acc;
	pjsip_tpselector tp_sel;
	pj_str_t target;
	pj_status_t status;
	dave_bool ret = dave_true;

	if(pData->dlg != NULL)
	{
		return dave_true;
	}

	acc = &(pjsua_var.acc[pData->acc_id]);

	target = pj_str((char *)(pData->request_uri));

	status = pjsip_dlg_create_uac(pjsip_ua_instance(), &(acc->cfg.id), &(acc->contact), &(acc->cfg.id), &target, &(pData->dlg));
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("dlg creat failed!");
		return dave_false;
	}

	pjsip_dlg_inc_lock(pData->dlg);

	pjsip_dlg_set_via_sent_by(pData->dlg, &acc->via_addr, acc->via_tp);

	pjsua_init_tpselector(acc->cfg.transport_id, &tp_sel);
	pjsip_dlg_set_transport(pData->dlg, &tp_sel);

    if(!pj_list_empty(&(acc->route_set)))
	{
		pjsip_dlg_set_route_set(pData->dlg, &(acc->route_set));
    }

    if(acc->cred_cnt)
	{
		pjsip_auth_clt_set_credentials(&(pData->dlg->auth_sess), acc->cred_cnt, acc->cred);
    }

	status = pjsip_evsub_create_uac(pData->dlg, &_evsub_subscribe_user, &_str_conference_event, PJSIP_EVSUB_NO_EVENT_ID, &(pData->sub));
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("sub creat failed!");
		ret = dave_false;
	}	
	else
	{
		pjsip_evsub_set_mod_data(pData->sub, pModule->id, pData);
	}

	pjsip_dlg_dec_lock(pData->dlg);

	return ret;
}

static dave_bool
_conf_subscribe_send(s8 *uri, ub uri_len)
{
	pjsip_tx_data *tdata;
	pj_status_t status;
	dave_bool ret = dave_true;

	if(_conf_mod_data.success == dave_false)
	{
		PARTYABNOR("subscribe init failed!");
		return dave_false;
	}

	dave_strcpy(_conf_mod_data.request_uri, uri, uri_len + 1);

	if(_conf_subscribe_register_data(&_mod_subscribe, &_conf_mod_data) == dave_false)
	{
		return dave_false;
	}

	pjsip_dlg_inc_lock(_conf_mod_data.dlg);

    status = pjsip_evsub_initiate(_conf_mod_data.sub, &pjsip_subscribe_method, SUBSCRIBE_DEFAULT_EXPIRES, &tdata);
	if(status != PJ_SUCCESS)
	{
		PARTYABNOR("subscribe data creat failed:%d!", status);
		ret = dave_false;
	}
	else
	{
		pjsua_process_msg_data(tdata, NULL);

		status = pjsip_evsub_send_request(_conf_mod_data.sub, tdata);
		if(status != PJ_SUCCESS)
		{
			PARTYABNOR("subscribe data send failed:%d!", status);
			ret = dave_false;
		}
		
	}

	pjsip_dlg_dec_lock(_conf_mod_data.dlg);

	return ret;
}

// =====================================================================

dave_bool
conf_subscribe_init(ThreadId owner, sb acc_id)
{
	_owner_thread_id = owner;

	_conf_mod_data.dlg = NULL;
	_conf_mod_data.sub = NULL;
	_conf_mod_data.acc_id = acc_id;

	_conf_mod_data.success = _conf_subscribe_register_mode(&_mod_subscribe);
	if(_conf_mod_data.success == dave_false)
	{
		_conf_subscribe_unregister_mode(&_mod_subscribe);
	}

	_conf_subscribe_cb = NULL;

	return _conf_mod_data.success;
}

void
conf_subscribe_exit(void)
{
	_conf_subscribe_unregister_mode(&_mod_subscribe);
}

dave_bool
conf_subscribe(s8 *request_uri, pjsip_cb_fun state_fun)
{
	_conf_subscribe_cb = state_fun;

	return _conf_subscribe_send(request_uri, dave_strlen(request_uri));
}

#endif

