/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __OSIP_SDP_H__
#define __OSIP_SDP_H__

sdp_message_t * osip_build_sdp(s8 *local_ip, s8 *rtp_port);
sdp_message_t * osip_load_sdp(osip_message_t *sip);

#endif