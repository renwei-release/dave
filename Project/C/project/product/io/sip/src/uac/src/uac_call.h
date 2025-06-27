/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_CALL_H__
#define __UAC_CALL_H__

RetCode uac_call(s8 *call_id_ptr, ub call_id_len, ThreadId owner_id, s8 *phone_number);

RetCode uac_bye(s8 *call_id_ptr, ub call_id_len, ThreadId owner_id, s8 *phone_number);

void uac_rtp(
	s8 *call_id, s8 *call_from, s8 *call_to,
	u32 ssrc,
	u8 payload_type, s8 *payload_ptr, ub payload_len);

#endif

