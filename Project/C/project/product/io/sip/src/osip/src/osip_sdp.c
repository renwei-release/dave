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
#include "osip_log.h"

static void
_osip_build_media(sdp_message_t *sdp, s8 *rtp_port, u8 media_format)
{
	switch(media_format)
	{
		case 0:
				sdp_message_m_media_add(sdp, "audio", rtp_port, NULL, "RTP/AVP 0");
				sdp_message_a_attribute_add(sdp, 0, "rtpmap", "0 PCMU/8000");	
			break;
		case 8:
				sdp_message_m_media_add(sdp, "audio", rtp_port, NULL, "RTP/AVP 8");
				sdp_message_a_attribute_add(sdp, 0, "rtpmap", "8 PCMA/8000");
			break;
		case 96:
				sdp_message_m_media_add(sdp, "audio", rtp_port, NULL, "RTP/AVP 96");
				sdp_message_a_attribute_add(sdp, 0, "rtpmap", "96 L16/16000/2");
			break;
		default:
				sdp_message_m_media_add(sdp, "audio", rtp_port, NULL, "RTP/AVP 8");
				sdp_message_a_attribute_add(sdp, 0, "rtpmap", "8 PCMA/8000");
			break;
	}
}

// =====================================================================

sdp_message_t *
osip_build_sdp(s8 *local_ip, s8 *rtp_port, u8 media_format)
{
    sdp_message_t *sdp;
    int err;
	s8 sess_id[16];

    err = sdp_message_init(&sdp);
    if (0 != err)
    {
        OSIPLOG("Failed to create <SDP> message err:%s", err);
        return NULL;
    }

    if (NULL == sdp)
    {
        OSIPLOG("Failed to create <SDP> message");
        return NULL;
    }

	dave_snprintf(sess_id, sizeof(sess_id), "%lu", t_rand());

    sdp_message_v_version_set(sdp, "0");
    sdp_message_o_origin_set(sdp, SIP_NAME, sess_id, sess_id, "IN", "IP4", local_ip);
    sdp_message_s_name_set(sdp, SIP_NAME); 
    sdp_message_c_connection_add(sdp, -1, "IN", "IP4", local_ip, NULL, NULL);
    sdp_message_t_time_descr_add(sdp, "0", "0");
	sdp_message_a_attribute_add(sdp, -1, "sendrecv", NULL);
	sdp_message_a_attribute_add(sdp, 0, "maxptime", "40");

	_osip_build_media(sdp, rtp_port, media_format);

    return sdp;
}
sdp_message_t *
osip_load_sdp(osip_message_t *sip)
{
	ub content_length;
	osip_list_iterator_t it;
	osip_body_t *body;
	sdp_message_t *sdp;
	int ret;

	if(sip->content_length == NULL)
	{
		return NULL;
	}

	content_length = stringdigital(sip->content_length->value);
	if(content_length == 0)
	{
		return NULL;
	}

	if(sip->content_type == NULL)
	{
		return NULL;
	}

	if((dave_strcmp(sip->content_type->type, "application") == dave_false)
		|| (dave_strcmp(sip->content_type->subtype, "sdp") == dave_false))
	{
		OSIPLOG("not sdp type:%s/%s", sip->content_type->type, sip->content_type->subtype);
		return NULL;
	}

	body = (osip_body_t *) osip_list_get_first(&(sip->bodies), &it);

	sdp_message_init(&sdp);

	ret = sdp_message_parse(sdp, body->body);
	if(ret != OSIP_SUCCESS)
	{
		OSIPLOG("sdp parse error:%d body:%s", ret, body->body);
		sdp_message_free(sdp);
		return NULL;
	}

	return sdp;
}

