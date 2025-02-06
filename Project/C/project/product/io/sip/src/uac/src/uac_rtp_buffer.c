/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_sip.h"
#include "dave_osip.h"
#include "dave_rtp.h"
#include "sip_signal.h"
#include "sip_call.h"
#include "uac_main.h"
#include "uac_cfg.h"
#include "uac_log.h"

static RetCode
_uac_rtp_buffer_recycle(void *ramkv, s8 *key)
{
	MBUF *pData = kv_del_key_ptr(ramkv, key);

	if(pData == NULL)
		return RetCode_empty_data;

	dave_mfree(pData);

	return RetCode_OK;
}

static void
_uac_rtp_buffer_load(UACRTPBuffer *pBuffer)
{
	MBUF *pre_buffer = kv_del_ub_ptr(pBuffer->pre_buffer_kv, (ub)(pBuffer->recv_sequence_number));

	while(pre_buffer != NULL)
	{
		pBuffer->recv_sequence_number ++;
	
		if((pBuffer->buffer_len + mlen(pre_buffer)) >= sizeof(pBuffer->buffer_ptr))
		{
			UACABNOR("Arithmetic error:%d/%d", pBuffer->buffer_len, mlen(pre_buffer));
		}
		else
		{
			pBuffer->buffer_len += dave_memcpy(&pBuffer->buffer_ptr[pBuffer->buffer_len], ms8(pre_buffer), mlen(pre_buffer));
		}

		dave_mfree(pre_buffer);

		pre_buffer = kv_del_ub_ptr(pBuffer->pre_buffer_kv, (ub)(pBuffer->recv_sequence_number));
	}
}

// =====================================================================

void
uac_rtp_buffer_init(UACRTPBuffer *pBuffer)
{
	s8 kv_name[64];

	t_lock_reset(&(pBuffer->buffer_pv));
	pBuffer->send_sequence_number = pBuffer->recv_sequence_number = 0;
	pBuffer->buffer_len = 0;
	dave_snprintf(kv_name, sizeof(kv_name), "rtpbuffer-%lx", pBuffer);
	pBuffer->pre_buffer_number = 0;
	pBuffer->pre_buffer_kv = kv_malloc(kv_name, 0, NULL);
}

void
uac_rtp_buffer_exit(UACRTPBuffer *pBuffer)
{
	kv_free(pBuffer->pre_buffer_kv, _uac_rtp_buffer_recycle);
}

MBUF *
uac_rtp_buffer(
	u16 *send_sequence_number,
	UACRTPBuffer *pBuffer,
	u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc, s8 *payload_ptr, ub payload_len)
{
	MBUF *payload_data = NULL;

	SAFECODEv1(pBuffer->buffer_pv, {
		if((pBuffer->recv_sequence_number == sequence_number)
			|| ((pBuffer->recv_sequence_number == 0) && (pBuffer->buffer_len == 0)))
		{
			pBuffer->recv_sequence_number = sequence_number + 1;
			if((pBuffer->buffer_len + payload_len) >= sizeof(pBuffer->buffer_ptr))
			{
				UACABNOR("Arithmetic error:%d/%d", pBuffer->buffer_len, payload_len);
			}
			else
			{
				pBuffer->buffer_len += dave_memcpy(&pBuffer->buffer_ptr[pBuffer->buffer_len], payload_ptr, payload_len);
			}

			_uac_rtp_buffer_load(pBuffer);

			if(pBuffer->buffer_len >= (sizeof(pBuffer->buffer_ptr) - 160))
			{
				payload_data = t_a2b_bin_to_mbuf(pBuffer->buffer_ptr, pBuffer->buffer_len);
				*send_sequence_number = pBuffer->send_sequence_number ++;
				pBuffer->buffer_len = 0;
			}
		}
		else
		{
			MBUF *pre_buffer = t_a2b_bin_to_mbuf(payload_ptr, payload_len);

			kv_add_ub_ptr(pBuffer->pre_buffer_kv, (ub)sequence_number, pre_buffer);
		}
	});

	return payload_data;
}

