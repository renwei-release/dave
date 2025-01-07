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
_osip_invite_request_line(osip_message_t *pInvite, s8 *server, s8 *port, s8 *username)
{
	osip_uri_t *uri = NULL;

    osip_message_set_version(pInvite, osip_strdup(SIP_VERSION_2_0));
    osip_message_set_method(pInvite, osip_strdup("INVITE"));

    osip_uri_init(&uri);
    osip_uri_set_scheme(uri, osip_strdup(SIP_SCHEME));
	osip_uri_set_username(uri, osip_strdup(username));
    osip_uri_set_host(uri, osip_strdup(server));
	osip_uri_set_port(uri, osip_strdup(port));

    osip_message_set_uri(pInvite, uri);
}

static void
_osip_invite_add_head(osip_message_t *pInvite, s8 *server, s8 *port, s8 *username, s8 *local_ip, s8 *local_port, s8 *call_number, ub cseq_number)
{
	s8 temp_buffer[256], branch[64];

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "sip:%s@%s", username, server);
    osip_message_set_from(pInvite, temp_buffer);
    osip_from_set_displayname(pInvite->from, osip_strdup(username));

    osip_from_set_tag(pInvite->from, osip_strdup(osip_tag(temp_buffer, sizeof(temp_buffer))));

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "sip:%s@%s", call_number, server);
    osip_message_set_to(pInvite, temp_buffer);

    osip_message_set_call_id(pInvite, osip_tag(temp_buffer, sizeof(temp_buffer)));

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "%d INVITE", cseq_number);
    osip_message_set_cseq(pInvite, temp_buffer);

    osip_message_set_max_forwards(pInvite, "70");
    osip_message_set_allow(pInvite, SIP_ALLOW);

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "sip:%s@%s:%s;transport=udp", username, local_ip, local_port);
    osip_message_set_contact(pInvite, temp_buffer);

    dave_snprintf(temp_buffer, sizeof(temp_buffer), "SIP/2.0/UDP %s:%s;rport;branch=%s", local_ip, local_port, osip_branch(branch, sizeof(branch)));
    osip_message_set_via(pInvite, temp_buffer);

	osip_message_set_user_agent(pInvite, SIP_NAME);
}

static void
_osip_invite_add_sdp(osip_message_t *pInvite, s8 *local_ip, s8 *rtp_port)
{
	sdp_message_t *sdp = NULL;
	char *sdp_string;
	int sdp_length;
	s8 temp_buffer[256];

	sdp = osip_build_sdp(local_ip, rtp_port);

	sdp_message_to_str(sdp, &sdp_string);
	sdp_length = dave_strlen(sdp_string);

	osip_message_set_content_type(pInvite, "application/sdp");
	digitalstring(temp_buffer, sizeof(temp_buffer), sdp_length);
	osip_message_set_content_length(pInvite, temp_buffer);
	osip_message_set_body(pInvite, sdp_string, sdp_length);

	sdp_message_free(sdp);
}

// =====================================================================

osip_message_t *
osip_invite(
	s8 *server, s8 *port, s8 *username,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port,
	s8 *call_number, ub cseq_number)
{
	osip_message_t *pInvite;

	osip_message_init(&pInvite);

	_osip_invite_request_line(pInvite, server, port, call_number);
	_osip_invite_add_head(pInvite, server, port, username, local_ip, local_port, call_number, cseq_number);
	_osip_invite_add_sdp(pInvite, rtp_ip, rtp_port);

	return pInvite;
}

