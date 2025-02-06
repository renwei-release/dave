/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __OSIP_INVITE_H__
#define __OSIP_INVITE_H__

osip_message_t * osip_invite(
	s8 *server, s8 *port, s8 *username,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port,
	s8 *call_number, ub cseq_number,
	u8 media_format);

#endif