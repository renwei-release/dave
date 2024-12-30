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
_osip_register_request_line(osip_message_t *pRegister, s8 *server, s8 *port, s8 *username)
{
	osip_uri_t *uri = NULL;

    osip_message_set_version(pRegister, osip_strdup(SIP_VERSION_2_0));
    osip_message_set_method(pRegister, osip_strdup("REGISTER"));

    osip_uri_init(&uri);
    osip_uri_set_scheme(uri, osip_strdup(SIP_SCHEME));
	osip_uri_set_username(uri, osip_strdup(username));
    osip_uri_set_host(uri, osip_strdup(server));
	osip_uri_set_port(uri, osip_strdup(port));

    osip_message_set_uri(pRegister, uri);
}

static void
_osip_register_add_head(osip_message_t *pRegister, s8 *server, s8 *port, s8 *username, s8 *local_ip, s8 *local_port, ub cseq_number)
{
	s8 temp_buffer[256], branch[64];

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "sip:%s@%s", username, server);
    osip_message_set_from(pRegister, temp_buffer);
    osip_from_set_displayname(pRegister->from, osip_strdup(username));

    osip_from_set_tag(pRegister->from, osip_strdup(osip_tag(temp_buffer, sizeof(temp_buffer))));

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "sip:%s@%s", username, server);
    osip_message_set_to(pRegister, temp_buffer);

    osip_message_set_call_id(pRegister, osip_tag(temp_buffer, sizeof(temp_buffer)));

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "%d REGISTER", cseq_number);
    osip_message_set_cseq(pRegister, temp_buffer);

    osip_message_set_max_forwards(pRegister, "70");
    osip_message_set_allow(pRegister, SIP_ALLOW);

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "sip:%s@%s:%s;ob", username, local_ip, local_port);
    osip_message_set_contact(pRegister, temp_buffer);

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "SIP/2.0/UDP %s:%s;rport;branch=%s", local_ip, local_port, osip_branch(branch, sizeof(branch)));
    osip_message_set_via(pRegister, temp_buffer);

	osip_message_set_user_agent(pRegister, SIP_NAME);
}

// =====================================================================

osip_message_t *
osip_register(
	s8 *server, s8 *port, s8 *username,
	s8 *local_ip, s8 *local_port,
	ub cseq_number)
{
	osip_message_t *pRegister;

	osip_message_init(&pRegister);

	_osip_register_request_line(pRegister, server, port, username);
	_osip_register_add_head(pRegister, server, port, username, local_ip, local_port, cseq_number);
	osip_message_set_content_length(pRegister, "0");

	return pRegister;
}

