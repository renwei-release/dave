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

static void
_osip_bye_request_line(osip_message_t *pBye, s8 *server_ip, s8 *server_port, s8 *username)
{
	osip_uri_t *uri = NULL;

    osip_message_set_version(pBye, osip_strdup(SIP_VERSION_2_0));
    osip_message_set_method(pBye, osip_strdup("BYE"));

    osip_uri_init(&uri);
    osip_uri_set_scheme(uri, osip_strdup(SIP_SCHEME));
	osip_uri_set_username(uri, osip_strdup(username));
    osip_uri_set_host(uri, osip_strdup(server_ip));
	osip_uri_set_port(uri, osip_strdup(server_port));

    osip_message_set_uri(pBye, uri);
}

static void
_osip_bye_add_head(osip_message_t *pBye, osip_from_t *from, osip_to_t *to, osip_call_id_t *call_id, ub cseq_number, s8 *local_ip, s8 *local_port)
{
	s8 temp_buffer[128], branch[64];

	osip_from_clone(from, &pBye->from);

	osip_to_clone(to, &pBye->to);

	osip_call_id_clone(call_id, &pBye->call_id);

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "%d BYE", cseq_number);
    osip_message_set_cseq(pBye, temp_buffer);

    osip_message_set_max_forwards(pBye, "70");

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "SIP/2.0/UDP %s:%s;rport;branch=%s", local_ip, local_port, osip_branch(branch, sizeof(branch)));
    osip_message_set_via(pBye, temp_buffer);

	osip_message_set_user_agent(pBye, SIP_NAME);
}

// =====================================================================

osip_message_t *
osip_bye(
	s8 *server_ip, s8 *server_port, s8 *username,
	s8 *local_ip, s8 *local_port,
	osip_from_t *from, osip_to_t *to, osip_call_id_t *call_id,
	ub cseq_number)
{
	osip_message_t *pBye;

	osip_message_init(&pBye);

	_osip_bye_request_line(pBye, server_ip, server_port, username);
	_osip_bye_add_head(pBye, from, to, call_id, cseq_number, local_ip, local_port);
	osip_message_set_content_length(pBye, "0");

	return pBye;
}

