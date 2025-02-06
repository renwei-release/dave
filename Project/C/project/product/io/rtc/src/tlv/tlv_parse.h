/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TLV_PARSE_H__
#define __TLV_PARSE_H__
#include "rtc_main.h"
#include "tlv_tag.h"

ub tlv_parse_find_end(RTCClient *pClient);

dave_bool tlv_parse_get_token(s8 *token_ptr, ub token_len, s8 *tlv_ptr, ub tlv_len);
void tlv_parse_get_tlv(RTCToken *pToken, u16 *my_data_serial, unsigned char **my_data_ptr, ub *my_data_length, s8 *tlv_ptr, ub tlv_len);

MBUF *tlv_parse_set_data(s8 *token, s8 *format, u16 serial, s8 *data_ptr, ub data_len);
MBUF * tlv_parse_set_close(void);

#endif

