/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __OSIP_TOOLS_H__
#define __OSIP_TOOLS_H__

ub osip_find_end_flag(s8 *data_ptr, ub data_len);

ub osip_content_length(s8 *data_ptr, ub data_len);

s8 * osip_get_from_user(osip_message_t *sip);

s8 * osip_get_to_user(osip_message_t *sip);

s8 * osip_get_call_id(osip_message_t *sip);

s8 * osip_get_cseq_method(osip_message_t *sip);

s8 * osip_get_cseq_number(osip_message_t *sip);

#endif

