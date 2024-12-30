/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __OSIP_BYE_H__
#define __OSIP_BYE_H__

osip_message_t * osip_bye(
	s8 *server_ip, s8 *server_port, s8 *username,
	s8 *local_ip, s8 *local_port,
	osip_from_t *from, osip_to_t *to, osip_call_id_t *call_id,
	ub cseq_number);

#endif
