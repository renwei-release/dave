/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "osip_param.h"
#include "osip_sdp.h"
#include "osip_log.h"

static osip_message_t *
_osip_create_ack(osip_message_t *request, osip_message_t *response)
{
	int i;
	osip_message_t *ack;

	i = osip_message_init(&ack);

	if (i != 0)
	{
		return NULL;
	}

	i = osip_from_clone(response->from, &(ack->from));

	if (i != 0)
	{
		OSIPLOG("");
		osip_message_free(ack);
		return NULL;
	}

	i = osip_to_clone(response->to, &(ack->to));

	if (i != 0)
	{
		OSIPLOG("");
		osip_message_free(ack);
		return NULL;
	}

	i = osip_call_id_clone(response->call_id, &(ack->call_id));

	if (i != 0)
	{
		OSIPLOG("");
		osip_message_free(ack);
		return NULL;
	}

	i = osip_cseq_clone(response->cseq, &(ack->cseq));

	if (i != 0)
	{
		OSIPLOG("");
		osip_message_free(ack);
		return NULL;
	}

	osip_free(ack->cseq->method);
	ack->cseq->method = osip_strdup("ACK");

	if (ack->cseq->method == NULL)
	{
		OSIPLOG("");
		osip_message_free(ack);
		return NULL;
	}

	ack->sip_method = (char *) osip_malloc(5);

	if (ack->sip_method == NULL)
	{
		OSIPLOG("");
		osip_message_free(ack);
		return NULL;
	}

	dave_sprintf(ack->sip_method, "ACK");
	ack->sip_version = osip_strdup(request->sip_version);

	if (ack->sip_version == NULL)
	{
		OSIPLOG("");
		osip_message_free(ack);
		return NULL;
	}

	ack->status_code = 0;
	ack->reason_phrase = NULL;

	i = osip_uri_clone(request->req_uri, &(ack->req_uri));

	if (i != 0)
	{
		OSIPLOG("");
		osip_message_free(ack);
		return NULL;
	}

	{
		osip_via_t *via;
		osip_via_t *orig_via;

		osip_message_get_via(request, 0, &orig_via);

		if (orig_via == NULL)
		{
			OSIPLOG("");
			osip_message_free(ack);
			return NULL;
		}

		i = osip_via_clone(orig_via, &via);

		if (i != 0)
		{
			OSIPLOG("");
			osip_message_free(ack);
			return NULL;
		}

		osip_list_add(&ack->vias, via, -1);
	}

	{
		int pos = 0;
		osip_route_t *route;
		osip_route_t *orig_route;

		while (!osip_list_eol(&request->routes, pos))
		{
			orig_route = (osip_route_t *) osip_list_get(&request->routes, pos);
			i = osip_route_clone(orig_route, &route);

			if (i != 0)
			{
				osip_message_free(ack);
				return NULL;
			}

			osip_list_add(&ack->routes, route, -1);
			pos++;
		}
	}

	if (response->status_code != 401 && response->status_code != 407)
	{
		if (osip_list_size(&request->authorizations) > 0)
		{
			i = osip_list_clone(&request->authorizations, &ack->authorizations, (int (*)(void *, void **)) & osip_authorization_clone);

			if (i != 0)
			{
				OSIPLOG("");
				osip_message_free(ack);
				return NULL;
			}
		}

		if (osip_list_size(&request->proxy_authorizations) > 0)
		{
			i = osip_list_clone(&request->proxy_authorizations, &ack->proxy_authorizations, (int (*)(void *, void **)) & osip_proxy_authorization_clone);

			if (i != 0)
			{
				OSIPLOG("");
				osip_message_free(ack);
				return NULL;
			}
		}
	}

	return ack;
}

// =====================================================================

osip_message_t *
osip_ack(osip_message_t *request, osip_message_t *response)
{
	osip_message_t *pAck = _osip_create_ack(request, response);

	osip_message_set_content_length(pAck, "0");

	return pAck;
}

